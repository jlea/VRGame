// Fill out your copyright notice in the Description page of Project Settings.

#include "ExtendedCharacter.h"
#include "Sound/SoundCue.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "DamageTypes/DamageType_Extended.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "VRGameState.h"
#include "Engine/DecalActor.h"
#include "Firearm.h"
#include "DrawDebugHelpers.h"
#include "Perception/PawnSensingComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AExtendedCharacter::AExtendedCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	SensingComponent->bEnableSensingUpdates = false;
	SensingComponent->bOnlySensePlayers = false;

	bPlayingDamageAnimation = false;
	bHasRagdolled = false;

	HeadshotForceMultiplier = 10000.0f;
	HeadshotMultiplier = 3.0f;
	MaxHealth = 100;
	bDead = false;

	HeadBone = TEXT("Head");

	BloodDecalMaxSprayDistance = 200.0f;
}

// Called when the game starts or when spawned
void AExtendedCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	auto GameState = Cast<AVRGameState>(GetWorld()->GetGameState());
	if (GameState)
	{
		GameState->AddSpawnedCharacter(this);
	}
}

// Called every frame
void AExtendedCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateCharacterBodyTwist(DeltaTime);
}

FVector AExtendedCharacter::GetTargetLocation(AActor* RequestedBy /* = nullptr */) const
{
	return GetMesh()->GetSocketLocation(HeadBone);
}

// Called to bind functionality to input
void AExtendedCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AExtendedCharacter::GetActorEyesViewPoint(FVector& Location, FRotator& Rotation) const
{
	const int32 HeadBoneIndex = GetMesh()->GetBoneIndex(HeadBone);
	const FTransform HeadBoneTransform = GetMesh()->GetBoneTransform(HeadBoneIndex);

	Location = HeadBoneTransform.GetLocation();
	Rotation = HeadBoneTransform.GetRotation().Rotator();
}

void AExtendedCharacter::FinishDamageAnimation()
{
	bPlayingDamageAnimation = false;
}

void AExtendedCharacter::UpdateCharacterBodyTwist(float DeltaSeconds)
{
	if (bDead || GetMesh()->IsSimulatingPhysics())
	{
		return;
	}

	/* Update the controller pitch and yaw for clients who aren't this.*/
	if (IsLocallyControlled() || Role == ROLE_Authority)
	{
		CurrentControllerYaw = GetControlRotation().Yaw;
		CurrentControllerPitch = GetControlRotation().Pitch;
	}

	FRotator AimRotation;
	AimRotation.Pitch = CurrentControllerPitch;
	AimRotation.Yaw = CurrentControllerYaw;

	FRotator RotationOffset = FRotator(0, -90, 0);

	bool bShouldTwist = true;
	if (bShouldTwist)
	{
		float YawThreshold = 90;
		AActor* RelativeActor = this;

		//Add the mesh relative rotation
		FRotator MeshRotation = GetMesh()->GetComponentRotation() - RotationOffset;

		AimPitchDelta = (MeshRotation - AimRotation).GetNormalized().Pitch;
		AimPitchDelta = FMath::Clamp<float>(AimPitchDelta, -90, 90);
		AimYawDelta = (MeshRotation - AimRotation).GetNormalized().Yaw;//
		AimYawDelta = FMath::Clamp<float>(AimYawDelta, -90, 90);

		//Calculate if we need to twist
		if ((AimYawDelta >= YawThreshold || AimYawDelta <= -YawThreshold))
		{
			FRotator NewMeshRotation;
			NewMeshRotation.Pitch = 0;
			NewMeshRotation.Roll = 0;
			NewMeshRotation.Yaw = CurrentControllerYaw + RotationOffset.Yaw;

			StoredMeshRotation = NewMeshRotation;
		}
	}

	//Set if we need to rotate the mesh
	if (!IsLockedToAnimation())
	{
		float InterpSpeed = 6.0f;

		//If we're moving, just rotate the mesh to our actor direction
		if (GetVelocity().Size() > 0.5f)
		{
			StoredMeshRotation = GetActorRotation() + RotationOffset;
			InterpSpeed = 24.0f;
		}

		//Rotate the character as needed
		FRotator InterpRotation = FMath::RInterpTo(GetMesh()->GetComponentRotation(), StoredMeshRotation, DeltaSeconds, InterpSpeed);
		GetMesh()->SetWorldRotation(InterpRotation);
	}
	else
	{
		GetMesh()->SetWorldRotation(GetActorRotation() + RotationOffset);
	}
}

bool AExtendedCharacter::ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	return Super::ShouldTakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

float AExtendedCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float BaseDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	float FinalDamage = BaseDamage;

	bool bHeadshot = false;

	FHitResult HitResult;
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageEvent = (const FPointDamageEvent*)&DamageEvent;
		if (PointDamageEvent)
		{
			HitResult = PointDamageEvent->HitInfo;

			if (!HeadBone.IsNone())
			{
				if (HitResult.BoneName == HeadBone)
				{
					bHeadshot = true;
				}
			}
		}
	}

	if (bHeadshot)
	{
		if (HeadshotEffects.Num() > 0)
		{
			UParticleSystem* HeadshotEffect = HeadshotEffects[FMath::RandRange(0, HeadshotEffects.Num() - 1)];
			if (HeadshotEffect)
			{
				UGameplayStatics::SpawnEmitterAttached(HeadshotEffect, GetMesh(), NAME_None, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation().GetInverse(), EAttachLocation::KeepWorldPosition);
			}
		}

		FinalDamage *= HeadshotMultiplier;
	}
	else
	{
		if (DamageEffects.Num() > 0)
		{
			UParticleSystem* DamageEffect = DamageEffects[FMath::RandRange(0, DamageEffects.Num() - 1)];
			if (DamageEffect)
			{
				UGameplayStatics::SpawnEmitterAttached(DamageEffect, GetMesh(), NAME_None, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation().GetInverse(), EAttachLocation::KeepWorldPosition);
			}
		}
	}

	// Trace behind to find where our blood might spawn
	if (BloodDecal)
	{
		FHitResult BloodTrace;
		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjectQueryParams.RemoveObjectTypesToQuery(ECC_Pawn);

		const FVector TraceStart = DamageCauser->GetActorLocation();
		const FVector TraceEnd = TraceStart + (DamageCauser->GetActorForwardVector() * BloodDecalMaxSprayDistance);
		GetWorld()->LineTraceSingleByObjectType(BloodTrace, TraceStart, TraceEnd, ObjectQueryParams);

		if (BloodTrace.IsValidBlockingHit())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;

			FRotator RandomDecalRotation = BloodTrace.ImpactNormal.Rotation().GetInverse();
			RandomDecalRotation.Add(-90.0f, 0.0f, 0.0f);
			//RandomDecalRotation.Yaw += 90.0f;

			const FVector SpawnLocation = BloodTrace.ImpactPoint;

			if(bHeadshot)
			{
				GetWorld()->SpawnActor<ADecalActor>(HeadshotDecal, SpawnLocation, RandomDecalRotation, SpawnParams);
			}
			else
			{
				GetWorld()->SpawnActor<ADecalActor>(BloodDecal, SpawnLocation, RandomDecalRotation, SpawnParams);
			}
		}
	}
	
	if(!bDead)
	{
		CurrentHealth -= FinalDamage;
		if (CurrentHealth <= 0.0f)
		{
			Kill(EventInstigator, DamageCauser, DamageEvent);
		}
		else
		{
			if (!bPlayingDamageAnimation)
			{
				PlayDialogueSound(PainSound);

				// See which direction our shot is attack from
				const FVector AttackDir = (HitResult.TraceEnd - HitResult.TraceStart).GetSafeNormal();
				const FVector ImpactDir = -AttackDir;
				const FRotator RotationDelta = (ImpactDir.Rotation() - GetMesh()->GetComponentRotation()).GetNormalized();

				if (EquippedFirearm)
				{
					const float YawThreshold = 30.0f;
					const float BackAttackThreshold = 90.0f;

					const bool bCenterAttack = FMath::Abs(RotationDelta.Yaw) < YawThreshold;
					const bool bBackAttack = FMath::Abs(RotationDelta.Yaw) >= BackAttackThreshold;
					const bool bLeftAttack = RotationDelta.Yaw >= YawThreshold && RotationDelta.Yaw < BackAttackThreshold;
					const bool bRightAttack = RotationDelta.Yaw <= -YawThreshold && RotationDelta.Yaw > -BackAttackThreshold;

					TArray<FDamageAnimation>	ValidDamageAnimations;
					for (auto DamageAnimation : EquippedFirearm->DamageAnimations)
					{
						switch (DamageAnimation.DamageDirection)
						{
						case EDamageDirection::Back:
							if (bBackAttack)
							{
								ValidDamageAnimations.Add(DamageAnimation);
							}
						case EDamageDirection::Front:
							if (bCenterAttack)
							{
								ValidDamageAnimations.Add(DamageAnimation);
							}
							break;
						case EDamageDirection::Left:
							if (bLeftAttack)
							{
								ValidDamageAnimations.Add(DamageAnimation);
							}
							break;
						case EDamageDirection::Right:
							if (bRightAttack)
							{
								ValidDamageAnimations.Add(DamageAnimation);
							}
							break;
						default:
							break;
						}
					}
				
					if (ValidDamageAnimations.Num() > 0)
					{
						int32 RandomAnimation = FMath::RandRange(0, ValidDamageAnimations.Num() - 1);
						if (ValidDamageAnimations.IsValidIndex(RandomAnimation))
						{
							float AnimDuration = PlayAnimMontage(ValidDamageAnimations[RandomAnimation].Animation);
							bPlayingDamageAnimation = true;

							//Schedule the ragdoll
							GetWorld()->GetTimerManager().SetTimer(TimerHandle_DamageAnimation, this, &ThisClass::FinishDamageAnimation, AnimDuration, false);
						}
					}
				}
			}
		}
	}
	else
	{
		// Character is dead already, see if we can stop their ragdoll
		if (!bHasRagdolled)
		{	
			CurrentHealth -= FinalDamage;

			if (CurrentHealth <= -100.0f)
			{
				Ragdoll();
			}
		}
	}

	return FinalDamage;
}

void AExtendedCharacter::PlayDeathAnimation()
{
	auto DeathAnimationsToPlay = DeathAnimations;
	if (EquippedFirearm)
	{
		DeathAnimationsToPlay.Append(EquippedFirearm->DeathAnimations);
	}

	int32 RandomAnimation = FMath::RandRange(0,DeathAnimationsToPlay.Num() - 1);
	if (DeathAnimationsToPlay.IsValidIndex(RandomAnimation))
	{
		float AnimDuration = PlayAnimMontage(DeathAnimationsToPlay[RandomAnimation]);

		//Schedule the ragdoll
		FTimerHandle Timer;
		GetWorld()->GetTimerManager().SetTimer(Timer, this, &AExtendedCharacter::Ragdoll, FMath::Max(0.1f, AnimDuration - 0.2f), false);
	}
	else
	{
		//Just ragdoll
		Ragdoll();
	}
}

bool AExtendedCharacter::ShouldRagdollOnDeath(FHitResult Hit, UDamageType_Extended* DamageType)
{
	if (DamageType && DamageType->bForceRagdoll)
	{
		return true;
	}

	if (!HeadBone.IsNone())
	{
		if (Hit.BoneName == HeadBone)
		{
			return true;
		}
	}

	auto DeathAnimationsToTest = DeathAnimations;
	if (EquippedFirearm)
	{
		DeathAnimationsToTest.Append(EquippedFirearm->DeathAnimations);
	}

	return DeathAnimationsToTest.Num() == 0;
}

void AExtendedCharacter::Kill(AController* Killer, AActor *DamageCauser, struct FDamageEvent const& DamageEvent)
{
	check(!bDead);

	UDamageType_Extended*	DamageType = nullptr;
	if (DamageEvent.DamageTypeClass)
	{
		DamageType = Cast<UDamageType_Extended>(DamageEvent.DamageTypeClass.GetDefaultObject());
	}

	if (GetController())
	{
		GetController()->UnPossess();
	}

	FHitResult HitResult;
	bool bWasHeadshot = false;

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageEvent = (const FPointDamageEvent*)&DamageEvent;
		if (PointDamageEvent)
		{
			HitResult = PointDamageEvent->HitInfo;

			if (!HeadBone.IsNone())
			{
				if (HitResult.BoneName == HeadBone)
				{
					bWasHeadshot = true;
				}
			}

			if (DamageType && DamageType->bSeverLimbs)
			{
				SeverLimb(HitResult);
			}
		}
	}

	if(bWasHeadshot)
	{
		
	}
	else
	{
		PlayDialogueSound(DeathSound);
	}

	if (ShouldRagdollOnDeath(HitResult, DamageType))
	{
		Ragdoll();

		if (bWasHeadshot)
		{
			GetMesh()->AddImpulse(DamageCauser->GetActorForwardVector() * HeadshotForceMultiplier, HeadBone);
		}
	}
	else
	{
		PlayDeathAnimation();
	}

	OnKilled(Killer, DamageCauser, DamageEvent);
	OnKilledDelegate.Broadcast(this, Killer, HitResult);

	if (EquippedFirearm)
	{
		EquippedFirearm->DetachFromCharacter();
	}

	bDead = true;

	SetActorTickEnabled(false);
}

void AExtendedCharacter::Ragdoll()
{
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->Deactivate();

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);

	bHasRagdolled = true;
}

void AExtendedCharacter::PlayDialogueSound(USoundCue* Sound)
{
	if (!Sound)
	{
		return;
	}

	//Stop our screams
	if (CurrentDialogue && CurrentDialogue->IsPlaying())
	{
		CurrentDialogue->Stop();
	}

	CurrentDialogue = UGameplayStatics::SpawnSoundAttached(Sound, GetMesh(), HeadBone);

	//Sound event
	MakeNoise(1.0f, this, GetActorLocation(), 2500.0f);
}

void AExtendedCharacter::Bleed(FName Bone)
{
	if (!GetMesh() || GetMesh()->GetBoneIndex(Bone) == INDEX_NONE)
	{
		return;
	}

	if (!BleedEffect)
	{
		return;
	}

	UGameplayStatics::SpawnEmitterAttached(BleedEffect, GetMesh(), Bone);
}

void AExtendedCharacter::SeverLimb(FHitResult Hit)
{
	//Already severed this limb
	if (SeveredLimbs.Contains(Hit.BoneName))
	{
		return;
	}

	//Find the bone name above the one we just hit
	int32 HitBoneIndex = GetMesh()->GetBoneIndex(Hit.BoneName);
	if (HitBoneIndex == INDEX_NONE)
	{
		return;
	}

	FName ParentBoneName = GetMesh()->GetParentBone(Hit.BoneName);
	if (ParentBoneName != NAME_None)
	{
		//Spawn some blood at the parent bone
		Bleed(ParentBoneName);
	}

	static FName CollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetCollisionProfileName(CollisionProfileName);
	GetMesh()->BreakConstraint(Hit.ImpactNormal, Hit.ImpactPoint, Hit.BoneName);
	SeveredLimbs.AddUnique(Hit.BoneName);

	//Add some blood to our limb
	Bleed(Hit.BoneName);

	//
	OnSeverLimb(Hit);
}

