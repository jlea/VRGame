// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spawner.generated.h"

class AExtendedCharacter;
class USkeletalMeshComponent;
class ATargetPoint;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSpawnerKilledEvent, ASpawner*, Spawner, AExtendedCharacter*, Character, AController*, Killer, const FHitResult&, HitResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpawnerAllKilledEvent, ASpawner*, Spawner);

UCLASS()
class VRTEST_API ASpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawner();

	virtual void Reset() override;
protected:
	// Called when the game starts or when spawned
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;

	virtual void PostInitProperties() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SpawnPawn();
protected:
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
	ATargetPoint* TargetLocation;

	UPROPERTY(EditInstanceOnly, Category = "Bullet Time")
	bool bTriggerBulletTimeOnDamage;

	UPROPERTY(EditInstanceOnly, Category = "Bullet Time")
	bool bTriggerBulletTimeOnKilled;

	UPROPERTY(BlueprintReadOnly)
	TArray<AExtendedCharacter*>	SpawnedPawns;

	int32 NumSpawned;
	int32 NumKilled;

	UPROPERTY()
	FTimerHandle TimerHandle_SpawnPawn;
public:

	UPROPERTY(BlueprintAssignable)
	FSpawnerKilledEvent OnPawnKilledDelegate;

	UPROPERTY(BlueprintAssignable)
	FSpawnerAllKilledEvent OnAllPawnsKilledDelegate;
};
