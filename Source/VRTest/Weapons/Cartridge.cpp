// Fill out your copyright notice in the Description page of Project Settings.

#include "Cartridge.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Magazine.h"
#include "Firearm.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Hand.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ACartridge::ACartridge()
{
	PrimaryActorTick.bCanEverTick = true;

	CasingMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("CasingMesh");
	CasingMeshComponent->SetSimulatePhysics(true);
	CasingMeshComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CasingMeshComponent->SetNotifyRigidBodyCollision(true);
	CasingMeshComponent->SetStaticMesh(CasingMeshFull);
	CasingMeshComponent->OnComponentHit.AddDynamic(this, &ACartridge::OnHit);

	bSpentRound = false;
	bHasBounced = false;

	RootComponent = CasingMeshComponent;
}

// Called when the game starts or when spawned
void ACartridge::BeginPlay()
{
	Super::BeginPlay();
}

bool ACartridge::CanGrab(const AHand* Hand)
{
	if (Super::CanGrab(Hand))
	{
		return !bSpentRound;
	}

	return false;
}

void ACartridge::OnDrop(AHand* Hand)
{
	Super::OnDrop(Hand);

	if (LoadableMagazine)
	{
		// Hand is holding us close to an empty magazine port, load us in
		LoadIntoMagazine(LoadableMagazine);
	}
}

void ACartridge::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LoadableMagazine = nullptr;

	if (AttachedHand)
	{
		// Find the closest weapon we have
		for (auto ClosestActor : AttachedHand->GetNearbyActors())
		{
			auto Firearm = Cast<AFirearm>(ClosestActor);
			if (Firearm)
			{
				if (!Firearm->bHasInternalMagazine)
				{
					continue;
				}

				auto PotentialMagazine = Firearm->GetLoadedMagazine();
				if (!PotentialMagazine)
				{
					continue;
				}

				if (!CanLoadIntoMagazine(PotentialMagazine))
				{
					continue;
				}

				if (PotentialMagazine->IsReadyToLoadCartridge(this))
				{
					LoadableMagazine = PotentialMagazine;
					break;
				}
			}
		}
	}
}

bool ACartridge::CanLoadIntoMagazine(AMagazine* Magazine)
{
	if (Magazine->CurrentAmmo == Magazine->AmmoCount)
	{
		return false;
	}

	if (!CompatibleMagazines.Contains(Magazine->GetClass()))
	{
		return false;
	}

	return true;
}

void ACartridge::LoadIntoMagazine(AMagazine* Magazine)
{
	if (!CanLoadIntoMagazine(Magazine))
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(this, LoadSound, GetActorLocation());

	Magazine->CurrentAmmo++;
	Destroy();
}

void ACartridge::OnEjected(bool bEmpty)
{
	if (bEmpty)
	{
		CasingMeshComponent->SetCollisionProfileName(TEXT("Debris"));
		CasingMeshComponent->SetStaticMesh(CasingMeshEmpty);

		SetLifeSpan(15.0f);
		bSpentRound = true;

		SetActorTickEnabled(false);
	}
}

void ACartridge::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bSpentRound)
	{
		return;
	}

	if (bHasBounced)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
	bHasBounced = true;
}

