// Fill out your copyright notice in the Description page of Project Settings.

#include "Firearm.h"
#include "Hand.h" 
#include "Projectile.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Magazine.h"
#include "ExtendedCharacter.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
AFirearm::AFirearm()
{
	bDropOnRelease = false;

	FirearmMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HandMesh");
	FirearmMesh->SetSimulatePhysics(true);

	MagazineCollisionBox = CreateDefaultSubobject<UBoxComponent>("MagazineCollisionBox");
	MagazineCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MagazineCollisionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	MagazineCollisionBox->SetupAttachment(FirearmMesh);

	//MagazinePreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>("MagazinePreviewMesh");
	//MagazinePreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FireRate = 600.0f;
	bAutomatic = true;
	bStartWithMagazine = true;

	LoadedMagazine = nullptr;

	bTriggerDown = false;
	HandAttachSocket = TEXT("WeaponMountSocket");
	MagazineAttachSocket = TEXT("MagazineAttachSocket");
	CharacterAttachSocket = TEXT("WeaponSocket");

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
	}
	else
	{
		EjectLoadedMagazine();
	}
}

// Called every frame
void AFirearm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bTriggerDown)
	{
		if (bAutomatic)
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
		if (!bAutomatic && bHasFired)
		{
			bHasFired = false;
		}
	}
}

bool AFirearm::CanGrab(const AHand* Hand)
{
	check(HandleBone.IsValid());

	if (AttachedCharacter)
	{
		return false;
	}

	return Super::CanGrab(Hand);
}

void AFirearm::OnBeginInteraction(AHand* Hand)
{
	if (Hand == AttachedHand)
	{
		if (LoadedMagazine && LoadedMagazine->CurrentAmmo == 0)
		{
			OnDryFire();
			return;
		}

		bTriggerDown = true;
		return;
	}

	Super::OnBeginInteraction(Hand);
}

void AFirearm::OnEndInteraction(AHand* Hand)
{
	if (Hand == AttachedHand)
	{
		bTriggerDown = false;
	}

	Super::OnEndInteraction(Hand);
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

	AttachedCharacter = nullptr;
}

bool AFirearm::CanFire()
{
	const float NextFireTime = (60 / FireRate);
	if (LastFireTime + NextFireTime > GetWorld()->GetTimeSeconds())
	{
		return false;
	}

	if (!LoadedMagazine)
	{
		return false;
	}
	else
	{
		if (LoadedMagazine->CurrentAmmo == 0)
		{
			return false;
		}
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

	if (FireAnimation)
	{
		FirearmMesh->GetAnimInstance()->Montage_Play(FireAnimation);
	}

	//GEngine->AddOnScreenDebugMessage(3, 1.0f, FColor::Red, FString::Printf(TEXT("%s: BANG!"), *GetName()), true, FVector2D(3.0f, 3.0f));

	const FTransform MuzzleTransform = FirearmMesh->GetSocketTransform(MuzzleBone);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	auto Projectile = GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleTransform, SpawnParams);

	LastFireTime = GetWorld()->GetTimeSeconds();

	bHasFired = true;

	LoadedMagazine->CurrentAmmo--;

	OnFire();
}

bool AFirearm::CanLoadMagazine(AMagazine* Magazine)
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

