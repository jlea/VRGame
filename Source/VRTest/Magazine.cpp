// Fill out your copyright notice in the Description page of Project Settings.

#include "Magazine.h"
#include "Firearm.h"
#include "Weapons/Cartridge.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Hand.h"

// Sets default values
AMagazine::AMagazine()
{
	MagazineMesh = CreateDefaultSubobject<UStaticMeshComponent>("MagazineMesh");
	MagazineMesh->SetSimulatePhysics(true);
	RootComponent = MagazineMesh;

	AmmoCount = 30;
	CurrentAmmo = AmmoCount;

	HandAttachSocket = TEXT("Magazine");

	bAttachToSocket = true;
	bDropOnRelease = true;
	bInteractable = true;

	InteractPriority = EInteractPriority::Low;
}

void AMagazine::BeginPlay()
{
	Super::BeginPlay();

	CurrentAmmo = AmmoCount;
}

bool AMagazine::CanGrab(const AHand* Hand)
{
	if (!bInteractable)
	{
		return false;
	}

	if (AttachedFirearm)
	{
		// Don't allow magazine pickup if we're in a gun on the ground
		if (!AttachedFirearm->GetAttachedHand())
		{
			return false;
		}

		// Don't allow pickup if we are in an empty weapon with some ammo, prefer to use the slide instead.
		if(AttachedFirearm->ChamberedRoundStatus == EChamberedRoundStatus::NoRound && CurrentAmmo > 0)
		{
			return false;
		}
	}	
	
	return Super::CanGrab(Hand);
}

void AMagazine::OnBeginPickup(AHand* Hand)
{
	if (AttachedFirearm)
	{
		AttachedFirearm->EjectLoadedMagazine();
	}

	Super::OnBeginPickup(Hand);
}

void AMagazine::OnDrop(AHand* Hand)
{
	Super::OnDrop(Hand);

	if (LoadableFirearm)
	{
		// Hand is holding us close to an empty magazine port, load us in
		LoadableFirearm->LoadMagazine(this);
	}
}

void AMagazine::OnMagazineLoaded(AFirearm* Firearm)
{
	UGameplayStatics::SpawnSoundAttached(LoadSound, MagazineMesh);

	MagazineMesh->SetSimulatePhysics(false);

	AttachToComponent(Firearm->FirearmMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Firearm->MagazineAttachSocket);
	MagazineMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Green, FString::Printf(TEXT("%s: Loading magazine"), *GetName()), true, FVector2D(3.0f, 3.0f));

	AttachedFirearm = Firearm;

	// Increase priority when loaded
	InteractPriority = EInteractPriority::Medium;

}

void AMagazine::OnMagazineEjected(AFirearm* Firearm)
{
	UGameplayStatics::SpawnSoundAttached(EjectSound, MagazineMesh);

	GEngine->AddOnScreenDebugMessage(3, 1.0f, FColor::Red, FString::Printf(TEXT("%s: Ejecting Magazine %s!"), *Firearm->GetName(), *GetName()), true, FVector2D(3.0f, 3.0f));

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);	
	MagazineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MagazineMesh->SetSimulatePhysics(true);

	AttachedFirearm = nullptr;

	InteractPriority = EInteractPriority::Low;
}

// Called every frame
void AMagazine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LoadableFirearm = nullptr;

	if (!AttachedFirearm)
	{
		if (AttachedHand)
		{
			// Find the closest weapon we have
			for (auto ClosestActor : AttachedHand->GetNearbyActors())
			{
				auto Firearm = Cast<AFirearm>(ClosestActor);
				if (Firearm)
				{
					if (!Firearm->IsCompatibleMagazine(this))
					{
						continue;
					}

					if (Firearm->IsReadyToLoadMagazine(this))
					{
						LoadableFirearm = Firearm;
						break;
					}
				}
			}
		}
	}
}

void AMagazine::ReplenishMagazine()
{
	CurrentAmmo = AmmoCount;
}

bool AMagazine::IsReadyToLoadCartridge(ACartridge* Cartridge)
{
	TSet<UPrimitiveComponent*>	OverlappingComponents;
	Cartridge->GetOverlappingComponents(OverlappingComponents);

	if (AttachedFirearm)
	{
		if (OverlappingComponents.Contains(AttachedFirearm->MagazineCollisionBox))
		{
			return true;
		}
	}
	else
	{
		if (OverlappingComponents.Contains(MagazineMesh))
		{
			return true;
		}
	}

	return false;
}