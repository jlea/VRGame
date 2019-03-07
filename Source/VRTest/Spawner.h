// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spawner.generated.h"

class AExtendedCharacter;
class USkeletalMeshComponent;

UCLASS()
class VRTEST_API ASpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawner();

protected:
	// Called when the game starts or when spawned
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;

	virtual void PostInitProperties() override;

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SpawnPawn();

	UFUNCTION()
	void PawnKilled(AExtendedCharacter* Character, AController* Killer, const FHitResult& HitResult);

	UFUNCTION(BlueprintImplementableEvent)
	void OnPawnKilled(AExtendedCharacter* Character, AController* Killer, const FHitResult& HitResult);

	UFUNCTION(BlueprintImplementableEvent)
	void OnAllPawnsKilled();

	UPROPERTY()
	USkeletalMeshComponent* SpawnerMeshPreview;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<AExtendedCharacter>	SpawnedCharacterClass;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	int32 MaxSpawns;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	int32 TeamId;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	float RespawnTime;

	UPROPERTY(EditInstanceOnly, Category = "Spawner")
	AActor* TargetLocation;

	UPROPERTY(BlueprintReadOnly)
	TArray<AExtendedCharacter*>	SpawnedPawns;

	int32 NumSpawned;
	int32 NumKilled;

	UPROPERTY()
	FTimerHandle TimerHandle_SpawnPawn;
};
