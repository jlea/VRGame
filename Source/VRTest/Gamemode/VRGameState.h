// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "VRGameState.generated.h"

class AInteractableActor;
class AExtendedCharacter;

/**
 * 
 */
UCLASS()
class VRTEST_API AVRGameState : public AGameStateBase
{
	friend class AVRGameMode;

	AVRGameState();

	GENERATED_BODY()

public:
	virtual void Tick(float DeltaSeconds) override;

	void AddSpawnedCharacter(AExtendedCharacter* Character);
	void AddInteractableActor(AInteractableActor* NewActor) { InteractableActors.AddUnique(NewActor); }

	void TriggerBulletTime(float Duration = 3.0f);

	UFUNCTION(BlueprintPure, Category = "VR")
	TArray<AInteractableActor*>	GetInteractableActors() { return InteractableActors; }

private:
	UPROPERTY()
	TArray<AExtendedCharacter*>	SpawnedCharacters;

	UPROPERTY()
	TArray<AInteractableActor*>	InteractableActors;

	UFUNCTION()
	void OnCharacterKilled(AExtendedCharacter* Character, AController* Killer, const FHitResult& HitEvent);
	
	UFUNCTION()
	void OnCharacterDamaged(AExtendedCharacter* Character, AController* Damager, const FHitResult& HitEvent);

	//////////////////////////////////////////////////////////////////////////
	//	Bullet time
protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Bullet Time")
	void OnBulletTimeBegin();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bullet Time")
	void OnBulletTimeFinish();

	UPROPERTY(BlueprintReadOnly, Category = "Bullet Time")
	bool bBulletTime;

	UPROPERTY(BlueprintReadOnly, Category = "Bullet Time")
	float CurrentTimeDilation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Time")
	float BulletTimeInterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Time")
	float BulletTimeModifier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Time")
	float BulletTimeDuration;

	UFUNCTION()
	void FinishBulletTime();

	UPROPERTY()
	FTimerHandle TimerHandle_BulletTime;

	//////////////////////////////////////////////////////////////////////////
	//	Scoring

	void ResetScores();

	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void BeginScoring();
	
	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void FinishScoring();

	UFUNCTION(BlueprintPure, Category = "Scoring")
	float GetTotalScore() const;

	float BeginScoringTimestamp;
	float FinishScoringTimestamp;

	UPROPERTY(BlueprintReadOnly, Category = "Scoring")
	TMap<FString,int32> NumShotsPerWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "Scoring")
	int32 NumPlayerShotsHit;

	UPROPERTY(BlueprintReadOnly, Category = "Scoring")
	int32 NumHeadshots;

	UPROPERTY(BlueprintReadOnly, Category = "Scoring")
	int32 NumDeadCharacters;

	UPROPERTY(BlueprintReadOnly, Category = "Scoring")
	float LastMatchDuration;
};
