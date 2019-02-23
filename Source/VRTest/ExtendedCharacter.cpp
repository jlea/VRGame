// Fill out your copyright notice in the Description page of Project Settings.

#include "ExtendedCharacter.h"
#include "Sound/SoundCue.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "DamageTypes/DamageType_Extended.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Firearm.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AExtendedCharacter::AExtendedCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bPlayingDamageAnimation = false;

	MaxHealth = 100;
	bDead = false;
}

// Called when the game starts or when spawned
void AExtendedCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
}

// Called every frame
void AExtendedCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateCharacterBodyTwist(DeltaTime);
}

// Called to bind functionality to input
void AExtendedCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
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
	if (bDead)
	{
		return false;
	}

	return Super::ShouldTakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

float AExtendedCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float BaseDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (!bDead)
	{
		CurrentHealth -= BaseDamage;
		if (CurrentHealth <= 0.0f)
		{
			Kill(EventInstigator, DamageCauser, DamageEvent);
		}
		else
		{
			if (!bPlayingDamageAnimation)
			{
				if (EquippedFirearm)
				{
					auto FirearmDamageAnimations = EquippedFirearm->DamageAnimations;

					int32 RandomAnimation = FMath::RandRange(0, FirearmDamageAnimations.Num() - 1);
					if (FirearmDamageAnimations.IsValidIndex(RandomAnimation))
					{
						float AnimDuration = PlayAnimMontage(FirearmDamageAnimations[RandomAnimation].Animation);
						bPlayingDamageAnimation = true;

						//Schedule the ragdoll
						GetWorld()->GetTimerManager().SetTimer(TimerHandle_DamageAnimation, this, &AExtendedCharacter::FinishDamageAnimation, AnimDuration, false);
					}
				}
			}
		}
	}

	return BaseDamage;
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

bool AExtendedCharacter::ShouldRagdollOnDeath(FHitResult Hit)
{
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

	GetController()->UnPossess();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->Deactivate();

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);

	FHitResult HitResult;
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageEvent = (const FPointDamageEvent*)&DamageEvent;
		if (PointDamageEvent)
		{
			HitResult = PointDamageEvent->HitInfo;

			if (DamageType && DamageType->bSeverLimbs)
			{
				SeverLimb(HitResult);
			}
		}
	}

	PlayDialogueSound(DeathSound);

	if (ShouldRagdollOnDeath(HitResult))
	{
		Ragdoll();
	}
	else
	{
		PlayDeathAnimation();
	}

	OnKilled(Killer, DamageCauser, DamageEvent);
	OnKilledDelegate.Broadcast(this, Killer);

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

	CurrentDialogue = UGameplayStatics::SpawnSoundAttached(Sound, GetMesh());

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

