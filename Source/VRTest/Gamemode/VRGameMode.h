// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VRGameMode.generated.h"

class AExtendedCharacter;
class ASpawner;
class AFirearm;

/**
 * 
 */
UCLASS()
class VRTEST_API AVRGameMode : public AGameModeBase
{
	GENERATED_BODY()

	AVRGameMode();

public:
	/** Restart the game, by default travel to the current map */
	UFUNCTION(BlueprintCallable, Category = "Game")
	virtual void RestartGame();

	virtual void ResetLevel() override;
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnPlayerFirearmFire(APlayerPawn* Player, AFirearm* Firearm);

	UFUNCTION()
	void OnPlayerKilled(APlayerPawn* Player, AController* Killer, const FHitResult& HitEvent);

	void SpawnWave();

	UFUNCTION()
	void OnAllPawnsKilledForSpawner(ASpawner* Spawner);

	UFUNCTION()
	void OnPawnKilledForSpawner(ASpawner* Spawner, AExtendedCharacter* Pawn, AController* Killer, const FHitResult& HitResult);

	UPROPERTY()
	TArray<APawn*>	SpawnedEnemies;

	UPROPERTY()
	TArray<ASpawner*>	Spawners;

	//////////////////////////////////////////////////////////////////////////
	//	Wave based
protected:
	void OnWaveFinished();
};
