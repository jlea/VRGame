// Fill out your copyright notice in the Description page of Project Settings.

#include "Spawner.h"
#include "ExtendedCharacter.h"

// Sets default values
ASpawner::ASpawner()
{
}

// Called when the game starts or when spawned
void ASpawner::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnPawns();
}

void ASpawner::SpawnPawns()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	auto SpawnedCharacter = GetWorld()->SpawnActor<AExtendedCharacter>(SpawnedCharacterClass, GetActorTransform(), SpawnParams);
}
