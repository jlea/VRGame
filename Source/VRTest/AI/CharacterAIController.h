// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CharacterAIController.generated.h"

/**
 * 
 */
UCLASS()
class VRTEST_API ACharacterAIController : public AAIController
{
	GENERATED_BODY()

	ACharacterAIController();

public:
	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual void Tick(float DeltaTime);
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn /* = true */) override;

	UFUNCTION(BlueprintCallable)
	void SetWantsFire(bool bEnabled);

	UPROPERTY(EditDefaultsOnly, Category = "AI Gameplay")
	float TurnInterpSpeed;

private:
	bool bShouldFire;
};
