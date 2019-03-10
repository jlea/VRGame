// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VRGameMode.generated.h"

class AExtendedCharacter;
class ASpawner;

/**
 * 
 */
UCLASS()
class VRTEST_API AVRGameMode : public AGameModeBase
{
	GENERATED_BODY()

	AVRGameMode();

public:
	virtual void ResetLevel() override;

	void SpawnWave();

	UPROPERTY()
	TArray<APawn*>	SpawnedEnemies;

	UPROPERTY()
	TArray<ASpawner*>	Spawners;
};
