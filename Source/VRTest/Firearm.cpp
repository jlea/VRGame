// Fill out your copyright notice in the Description page of Project Settings.

#include "Firearm.h"
#include "Hand.h" 
#include "Projectile.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Magazine.h"
#include "PlayerPawn.h"
#include "ExtendedCharacter.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Weapons/Cartridge.h"
#include "Components/BoxComponent.h"

// Sets default values
AFirearm::AFirearm()
{
	bHasSlidBack = false;
	SlideStartSocket = TEXT("SlideStart");
	SlideEndSocket = TEXT("SlideEnd");

	bDropOnRelease = false;
	bEjectRoundOnFire = true;

	FirearmMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HandMesh");
	FirearmMesh->SetSimulatePhysics(true);
	FirearmMesh->SetCollisionProfileName(TEXT("Weapon"));

	MagazineCollisionBox = CreateDefaultSubobject<UBoxComponent>("MagazineCollisionBox");
	MagazineCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MagazineCollisionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	MagazineCollisionBox->SetupAttachment(FirearmMesh);

	MagazinePreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>("MagazinePreviewMesh");
	MagazinePreviewMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	MagazinePreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MagazinePreviewMesh->SetupAttachment(MagazineCollisionBox);

	BulletsPerShot = 1;
	FireRate = 600.0f;
	Spread = 0.0f;
	AmmoLoadType = EFirearmAmmoLoadType::Automatic;

	ChamberedRoundStatus = EChamberedRoundStatus::NoRound;
	bStartWithMagazine = true;
	bHasInternalMagazine = false;
	LoadedMagazine = nullptr;

	AmmoPreviewStatus = EAmmoPreviewStatus::None;

	bTriggerDown = false;
	HandAttachSocket = TEXT("WeaponMountSocket");
	MagazineAttachSocket = TEXT("MagazineAttachSocket");
	CharacterAttachSocket = TEXT("WeaponSocket");
	ShellAttachSocket = TEXT("ShellEjectSocket");

	CartridgeEjectVelocity = 400.0f;

	PickupBones.Add(TEXT("b_gun_Root"));
	PickupBones.Add(TEXT("b_gun_trigger"));
	PickupBones.Add(TEXT("b_gun_stock"));

	RootComponent = FirearmMesh;
}

// Called when the game starts or when spawned
void AFirearm::BeginPlay()
{
	Super::BeginPlay();

	if (!MagazineBone.IsNone())
	{
		int32 BoneIndex = FirearmMesh->GetBoneIndex(MagazineBone);
		FirearmMesh->HideBone(BoneIndex, PBO_None);
	}

	if(bStartWithMagazine)
	{
		check(DefaultMagazineClass);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		auto NewMagazine = GetWorld()->SpawnActor<AMagazine>(DefaultMagazineClass, GetActorTransform(), SpawnParams);
		LoadMagazine(NewMagazine);
		LoadRoundFromMagazine();

		if (bHasInternalMagazine)
		{
			NewMagazine->bInteractable = false;
		}
	}
	else
	{
		EjectLoadedMagazine();
	}

	SetAmmoPreviewStatus(EAmmoPreviewStatus::None);
}

// Called every frame
void AFirearm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto OldPreviewStatus = AmmoPreviewStatus;
	EAmmoPreviewStatus NewAmmoPreviewStatus = EAmmoPreviewStatus::None;

	if (AttachedHand)
	{
		APlayerPawn* Player = AttachedHand->GetPlayerPawn();
		if (Player)
		{
			TArray<AInteractableActor*>	HeldActors;

			HeldActors.Add(Player->LeftHand->GetInteractingActor());
			HeldActors.Add(Player->RightHand->GetInteractingActor());

			for (auto HeldActor : HeldActors)
			{
				if (!HeldActor)
				{
					continue;
				}

				auto HeldCartridge = Cast<ACartridge>(HeldActor);
				if (HeldCartridge)
				{
					auto LoadedMagazine = GetLoadedMagazine();;
					if (LoadedMagazine && HeldCartridge->CanLoadIntoMagazine(LoadedMagazine))
					{
						NewAmmoPreviewStatus = EAmmoPreviewStatus::OutOfRange;

						if (LoadedMagazine->IsReadyToLoadCartridge(HeldCartridge))
						{
							NewAmmoPreviewStatus = EAmmoPreviewStatus::WithinRange;
						}
					}
				}

				auto HeldMagazine = Cast<AMagazine>(HeldActor);
				if (HeldMagazine)
				{
					if (IsCompatibleMagazine(HeldMagazine))
					{
						// See if we are ready to load it
						NewAmmoPreviewStatus = EAmmoPreviewStatus::OutOfRange;

						if (IsReadyToLoadMagazine(HeldMagazine))
						{
							NewAmmoPreviewStatus = EAmmoPreviewStatus::WithinRange;
						}
					}
				}
			}
		}
	}

	if (NewAmmoPreviewStatus != OldPreviewStatus)
	{
		SetAmmoPreviewStatus(NewAmmoPreviewStatus);
	}
	
	if (bTriggerDown)
	{
		if (AmmoLoadType == EFirearmAmmoLoadType::Automatic)
		{
			if (CanFire())
			{
				Fire();
			}
		}
		else
		{
			// Don't fire again until we release our trigger
			if (!bHasFired)
			{
				if (CanFire())
				{
					Fire();
				}
			}
		}
	}
	else
	{
		// Single action weapon
		if (AmmoLoadType != EFirearmAmmoLoadType::Automatic && bHasFired)
		{
			bHasFired = false;
		}
	}

	if (InteractingHand && InteractingHand != AttachedHand)
	{
		const FVector StartLocation = FirearmMesh->GetSocketLocation(SlideStartSocket);
		const FVector EndLocation = FirearmMesh->GetSocketLocation(SlideEndSocket);
		const FVector HandLocation = InteractingHand->GetHandSelectionOrigin();
		const FVector ClosestPoint = FMath::ClosestPointOnLine(StartLocation, EndLocation, HandLocation);

		//DrawDebugSphere(GetWorld(), ClosestPoint, 5.0f, 8, FColor::Red, false, 0.1f);

		const float DistanceToStart = FVector::Dist2D(ClosestPoint, StartLocation);
		const float DistanceToEnd = FVector::Dist2D(ClosestPoint, EndLocation);

		const float DistanceBetweenSockets = FVector::Dist2D(StartLocation, EndLocation);
		const float Ratio = 1.0f - (DistanceToStart / DistanceBetweenSockets);

		SlideProgress = Ratio;

		// Pumping it?
		if (!bHasSlidBack)
		{
			if (SlideProgress >= 1.0)
			{
				EjectRound();
				OnSlideBack();

				bHasSlidBack = true;
			}
		}
		else
		{
			if (SlideProgress <= 0.0f)
			{
				bHasSlidBack = false;

				LoadRoundFromMagazine();
				OnSlideForward();
			}
		}
	}
}

void AFirearm::SetAmmoPreviewStatus(EAmmoPreviewStatus NewPreviewStatus)
{
	AmmoPreviewStatus = NewPreviewStatus;

	if (AmmoPreviewStatus == EAmmoPreviewStatus::None)
	{
		MagazinePreviewMesh->SetVisibility(false, true);
	}
	else
	{
		MagazinePreviewMesh->SetVisibility(true, true);
	}

	OnNearbyHeldAmmoChanged(AmmoPreviewStatus);
}

bool AFirearm::CanGrab(const AHand* Hand)
{
	if (AttachedCharacter)
	{
		return false;
	}

	return Super::CanGrab(Hand);
}

void AFirearm::OnBeginInteraction(AHand* Hand)
{
	Super::OnBeginInteraction(Hand);

	if (Hand == AttachedHand)
	{
		if (ChamberedRoundStatus == EChamberedRoundStatus::NoRound || ChamberedRoundStatus == EChamberedRoundStatus::Spent)
		{
			UGameplayStatics::PlaySoundAtLocation(this, DryFireSound, FirearmMesh->GetSocketLocation(MuzzleBone));
			OnDryFire();
			return;
		}

		bTriggerDown = true;
	}
}

void AFirearm::OnEndInteraction(AHand* Hand)
{
	Super::OnEndInteraction(Hand);

	if (Hand == AttachedHand)
	{
		bTriggerDown = false;
	}
}

void AFirearm::OnBeginPickup(AHand* Hand)
{
	Super::OnBeginPickup(Hand);

	AttachToComponent(Hand->GetHandMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandAttachSocket);

	if (Hand->HandType == EControllerHand::Left)
	{
		FRotator NewRelativeRotation = RootComponent->RelativeRotation;
		NewRelativeRotation.Roll += 180.0f;

		SetActorRelativeRotation(NewRelativeRotation);
	}

	OnFirearmPickedUp.Broadcast(this);
}

void AFirearm::OnDrop(AHand* Hand)
{
	Super::OnDrop(Hand);

	bTriggerDown = false;

	OnFirearmDropped.Broadcast(this);
}

bool AFirearm::AttachToCharacter(AExtendedCharacter* NewCharacter)
{
	if (!NewCharacter)
	{
		return false;
	}

	FirearmMesh->SetSimulatePhysics(false);
	AttachToComponent(NewCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, CharacterAttachSocket);
	SetActorEnableCollision(false);

	if (LoadedMagazine)
	{
		LoadedMagazine->SetActorEnableCollision(false);
	}

	AttachedCharacter = NewCharacter;
	AttachedCharacter->EquippedFirearm = this;

	return true;
}

void AFirearm::DetachFromCharacter()
{
	if (!AttachedCharacter)
	{
		return;
	}

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	FirearmMesh->SetSimulatePhysics(true);
	SetActorEnableCollision(true);

	if (LoadedMagazine)
	{
		LoadedMagazine->SetActorEnableCollision(true);
	}

	AttachedCharacter->EquippedFirearm = nullptr;
	AttachedCharacter = nullptr;
}

bool AFirearm::CanFire()
{
	const float NextFireTime = (60 / FireRate);
	if (LastFireTime + NextFireTime > GetWorld()->GetTimeSeconds())
	{
		return false;
	}

	if (ChamberedRoundStatus != EChamberedRoundStatus::Fresh)
	{
		return false;
	}

	return true;
}

void AFirearm::Fire()
{
	check(FireRate > 0);
	check(ProjectileClass);

	if (AttachedHand)
	{
		AttachedHand->GetPlayerController()->PlayHapticEffect(RecoilEffect, AttachedHand->HandType);
	}

	if (FireAnimation.Num() > 0)
	{
		FirearmMesh->GetAnimInstance()->Montage_Play(FireAnimation[FMath::RandRange(0, FireAnimation.Num() - 1)]);
	}

	if (FireAnimationCharacter.Num() > 0)
	{
		if (AttachedCharacter)
		{
			AttachedCharacter->PlayAnimMontage(FireAnimationCharacter[FMath::RandRange(0, FireAnimationCharacter.Num() - 1)]);
		}
	}

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, FirearmMesh->GetSocketLocation(MuzzleBone));
	}

	if (FireParticle)
	{
		UGameplayStatics::SpawnEmitterAttached(FireParticle, FirearmMesh, MuzzleBone);
	}

	FTransform MuzzleTransform = FirearmMesh->GetSocketTransform(MuzzleBone);

	// If attached to a bot, use their control rotation instead of bone rotation
	if (AttachedCharacter)
	{
		MuzzleTransform.SetRotation(AttachedCharacter->GetController()->GetControlRotation().Quaternion());
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	for (int i = 0; i < BulletsPerShot; i++)
	{
		FVector SpawnLocation = MuzzleTransform.GetLocation();
		FRotator SpawnRotation = MuzzleTransform.GetRotation().Rotator() + FRotator(FMath::RandRange(-Spread, Spread), FMath::RandRange(-Spread, Spread), 0.0f);

		auto Projectile = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	}

	LastFireTime = GetWorld()->GetTimeSeconds();

	bHasFired = true;
	ChamberedRoundStatus = EChamberedRoundStatus::Spent;

	if (bEjectRoundOnFire)
	{
		EjectRound();
	}

	OnFire();

	// Load the next round in if we have one
	if (AmmoLoadType == EFirearmAmmoLoadType::Automatic || AmmoLoadType == EFirearmAmmoLoadType::SemiAutomatic)
	{
		LoadRoundFromMagazine();
	}

	// Make some noise after we fire
	if (AttachedCharacter)
	{
		MakeNoise(1.0f, AttachedCharacter);
		AttachedCharacter->MakeNoise(1.0f);
	}
	else if (AttachedHand && AttachedHand->GetPlayerPawn())
	{
		MakeNoise(1.0f, AttachedHand->GetPlayerPawn());
	}
}

void AFirearm::LoadRoundFromMagazine()
{
	if (!LoadedMagazine)
	{
		return;
	}

	if (LoadedMagazine && LoadedMagazine->CurrentAmmo <= 0)
	{
		return;
	}

	LoadedMagazine->CurrentAmmo--;
	ChamberedRoundStatus = EChamberedRoundStatus::Fresh;
}

void AFirearm::EjectRound()
{
	if (ChamberedRoundStatus != EChamberedRoundStatus::NoRound)
	{
		if (CartridgeClass)
		{
			FTransform SpawnTransform = FirearmMesh->GetSocketTransform(ShellAttachSocket);

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			auto Shell = GetWorld()->SpawnActor<ACartridge>(CartridgeClass, SpawnTransform, SpawnParams);

			Shell->OnEjected(ChamberedRoundStatus == EChamberedRoundStatus::Spent);
			Shell->GetMesh()->AddImpulse(-Shell->GetActorForwardVector() * CartridgeEjectVelocity, NAME_None, true);
		}
	}

	ChamberedRoundStatus = EChamberedRoundStatus::NoRound;
}

bool AFirearm::IsCompatibleMagazine(AMagazine* Magazine)
{
	if(!Magazine)
	{
		return false;
	}

	if(LoadedMagazine)
	{
		return false;
	}

	if (!Magazine->CompatibleFirearms.Contains(GetClass()))
	{
		return false;
	}

	return true;
}

bool AFirearm::IsReadyToLoadMagazine(AMagazine* Magazine)
{
	TSet<UPrimitiveComponent*>	OverlappingComponents;
	Magazine->GetOverlappingComponents(OverlappingComponents);

	if (OverlappingComponents.Contains(MagazineCollisionBox))
	{
		return true;
	}

	return false;
}

int32 AFirearm::GetLoadedRounds()
{
	int32 Rounds = 0;

	if (ChamberedRoundStatus == EChamberedRoundStatus::Fresh)
	{
		Rounds++;
	}

	if (LoadedMagazine)
	{
		Rounds += LoadedMagazine->CurrentAmmo;
	}

	return Rounds;
}

void AFirearm::LoadMagazine(AMagazine* NewMagazine)
{
	check(!LoadedMagazine);

	LoadedMagazine = NewMagazine;
	LoadedMagazine->OnMagazineLoaded(this);
	OnMagazineLoaded.Broadcast(this, LoadedMagazine);
}

bool AFirearm::EjectLoadedMagazine()
{
	if (!LoadedMagazine)
	{
		return false;
	}

	LoadedMagazine->OnMagazineEjected(this);
	OnMagazineEjected.Broadcast(this, LoadedMagazine);

	LoadedMagazine = nullptr;
	return true;
}

