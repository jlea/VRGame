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
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn /* = true */) override;

	UPROPERTY(EditDefaultsOnly, Category = "AI Gameplay")
	float TurnInterpSpeed;
};
