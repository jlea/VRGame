// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CharacterAIController.generated.h"

USTRUCT(BlueprintType)
struct FAITargetMemory
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadOnly)
		APawn* TargetPawn;

	UPROPERTY(BlueprintReadOnly)
		float LastSeenTimestamp;

	UPROPERTY(BlueprintReadOnly)
		FVector LastSeenPosition;

	UPROPERTY(BlueprintReadOnly)
		FVector LastSeenVelocity;

	UPROPERTY(BlueprintReadOnly)
		bool bHasLOS;

	UPROPERTY(BlueprintReadOnly)
		bool bIsAlive;

	/** defaults */
	FAITargetMemory()
	{
		TargetPawn = nullptr;
		bHasLOS = false;
		bIsAlive = false;
	}
};

/**
 * 
 */
UCLASS()
class VRTEST_API ACharacterAIController : public AAIController
{
	GENERATED_BODY()

	ACharacterAIController();

public:

	UFUNCTION(BlueprintPure, Category = "AI")
	FGenericTeamId GetTeamId() const { return GetGenericTeamId(); }
	
	virtual bool LineOfSightTo(const AActor* Other, FVector ViewPoint = FVector(ForceInit), bool bAlternateChecks = false) const override;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void Tick(float DeltaTime);
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn /* = true */) override;
	virtual FVector GetFocalPointOnActor(const AActor *Actor) const;

	UFUNCTION(BlueprintCallable)
	void SetWantsFire(bool bEnabled);


	/* Update our targets position */
	virtual void UpdateMemory();
	virtual void UpdateTargetBlackboard();

	/* Check if we have LOS to our current target */
	virtual void DoVisibilityCheck();

	/* Conditional checks for selecting enemies */
	virtual class APawn* GetTargetPawn() { return TargetMemory.TargetPawn; }

	virtual bool ShouldSetAsEnemy(APawn* Enemy);
	virtual void SetNewEnemy(APawn* NewEnemy);

	/* Flag our memory as having a direct, traced LOS to the target */
	virtual void SetLOSToTarget(bool bHasLOS);

	/* A Lower numer is a higher priority for selection */
	virtual int32 GetPriorityForTarget(class APawn* Target);

	UFUNCTION()
	virtual void OnHearNoise(APawn *OtherActor, const FVector &Location, float Volume);

	UFUNCTION()
	virtual void OnSeePawn(APawn *OtherPawn);

	UPROPERTY(BlueprintReadOnly, Category = Memory)
	FAITargetMemory TargetMemory;

	/* Called when gained/lost LOS to our current target. CurrentTarget is still valid */
	UFUNCTION(BlueprintImplementableEvent)
	void OnGainedLOS();

	UFUNCTION(BlueprintImplementableEvent)
	void OnLostLOS();

	UFUNCTION(BlueprintImplementableEvent)
	void OnTargetKilled();

	UPROPERTY()
	FTimerHandle TimerHandle_UpdateTimer;

	UPROPERTY(EditDefaultsOnly, Category = "AI Gameplay")
	float TurnInterpSpeed;

	/* Name of the corresponding key in our blackboard asset */
	UPROPERTY(EditDefaultsOnly, Category = "Behavior Tree")
	FName TargetEnemyKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "Behavior Tree")
	FName TargetDistanceKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "Behavior Tree")
	FName TargetHasLOSKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "Behavior Tree")
	FName TargetAliveKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "Behavior Tree")
	FName TargetLastSeenPositionKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "Behavior Tree")
	FName TargetLastSeenVelocityKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "Behavior Tree")
	FName TargetLastSeenTimeKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "Suppression")
	float Suppression;
	
private:
	bool bShouldFire;
};
