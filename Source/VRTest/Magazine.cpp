// Fill out your copyright notice in the Description page of Project Settings.

#include "Magazine.h"
#include "Firearm.h"
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

	HandAttachSocket = TEXT("MagazineMountSocket");

	bDropOnRelease = true;
}

void AMagazine::BeginPlay()
{
	Super::BeginPlay();

	CurrentAmmo = AmmoCount;
}

bool AMagazine::CanGrab(const AHand* Hand)
{
	if (AttachedFirearm)
	{
		if (!AttachedFirearm->GetAttachedHand())
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

	AttachToComponent(Hand->GetHandMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandAttachSocket);
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
}

void AMagazine::OnMagazineEjected(AFirearm* Firearm)
{
	UGameplayStatics::SpawnSoundAttached(EjectSound, MagazineMesh);

	GEngine->AddOnScreenDebugMessage(3, 1.0f, FColor::Red, FString::Printf(TEXT("%s: Ejecting Magazine %s!"), *Firearm->GetName(), *GetName()), true, FVector2D(3.0f, 3.0f));

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);	
	MagazineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MagazineMesh->SetSimulatePhysics(true);

	AttachedFirearm = nullptr;
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
					if (!Firearm->CanLoadMagazine(this))
					{
						continue;
					}

					TSet<UPrimitiveComponent*>	OverlappingComponents;
					GetOverlappingComponents(OverlappingComponents);

					if(OverlappingComponents.Contains(Firearm->MagazineCollisionBox))
					{
						LoadableFirearm = Firearm; 

						DrawDebugSphere(GetWorld(), Firearm->MagazineCollisionBox->GetComponentLocation(), 5.0f, 16, FColor::Green, false, 0.1f, SDPG_World, 0.25f);
						//DrawDebugLine(GetWorld(), AttachedHand->GetHandSelectionOrigin(), Firearm->FirearmMesh->GetBoneLocation(ClosestBone), FColor::Green, false, 0.1f, SDPG_World, 0.35f);
						break;
					}
				}
			}
		}
	}
}

