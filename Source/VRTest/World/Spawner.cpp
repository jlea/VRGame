// Fill out your copyright notice in the Description page of Project Settings.

#include "Spawner.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Character/ExtendedCharacter.h"

// Sets default values
ASpawner::ASpawner()
{
	NumSpawned = 0;
	MaxSpawns = 5;
	RespawnTime = 5.0f;

	bHidden = true;

	SpawnerMeshPreview = CreateDefaultSubobject<USkeletalMeshComponent>("HandMesh");
	SpawnerMeshPreview->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = SpawnerMeshPreview;
	TeamId = 1;
}

void ASpawner::Reset()
{
	NumSpawned = 0;
	NumKilled = 0;

	for (auto SpawnedPawn : SpawnedPawns)
	{
		if (!SpawnedPawn)
		{
			continue;
		}

		SpawnedPawn->Destroy();
	}

	SpawnedPawns.Empty();
}

void ASpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (SpawnedCharacterClass)
	{
		auto ClassCDO = SpawnedCharacterClass.GetDefaultObject();
		if (ClassCDO)
		{
			SpawnerMeshPreview->SetSkeletalMesh(ClassCDO->GetMesh()->SkeletalMesh);
		}
	}
}

// Called when the game starts or when spawned
void ASpawner::BeginPlay()
{
	Super::BeginPlay();
}

void ASpawner::PostInitProperties()
{
	Super::PostInitProperties();
}

void ASpawner::SpawnPawn()
{
	check(SpawnedCharacterClass);

	if (MaxSpawns > 0)
	{
		if (NumSpawned >= MaxSpawns)
		{
			return;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	auto SpawnedCharacter = GetWorld()->SpawnActorDeferred<AExtendedCharacter>(SpawnedCharacterClass, GetActorTransform(), this);
	if (SpawnedCharacter)
	{
		SpawnedCharacter->TeamId = TeamId;
		SpawnedCharacter->OnKilledDelegate.AddDynamic(this, &ThisClass::PawnKilled);

		UGameplayStatics::FinishSpawningActor(SpawnedCharacter, GetActorTransform());

		auto AIController = Cast<AAIController>(SpawnedCharacter->GetController());
		if (AIController && TargetLocation)
		{
			AIController->MoveToActor(TargetLocation);
		}

		SpawnedPawns.Add(SpawnedCharacter);

		NumSpawned++;
	}
}

void ASpawner::PawnKilled(AExtendedCharacter* Character, AController* Killer, const FHitResult& HitResult)
{
	NumKilled++;

	OnPawnKilled(Character, Killer, HitResult);
	OnPawnKilledDelegate.Broadcast(this, Character, Killer, HitResult);

	if (NumKilled == MaxSpawns)
	{
		OnAllPawnsKilled();
		OnAllPawnsKilledDelegate.Broadcast(this);
	}
	else
	{
		if (RespawnTime > 0.0f)
		{
			// Queue up another
			GetWorldTimerManager().SetTimer(TimerHandle_SpawnPawn, this, &ThisClass::SpawnPawn, RespawnTime);
		}
		else
		{
			SpawnPawn();
		}
	}
}
