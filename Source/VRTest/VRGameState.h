// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "VRGameState.generated.h"

class AInteractableActor;

/**
 * 
 */
UCLASS()
class VRTEST_API AVRGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	void AddInteractableActor(AInteractableActor* NewActor) { InteractableActors.AddUnique(NewActor); }

	UFUNCTION(BlueprintPure, Category = "VR")
	TArray<AInteractableActor*>	GetInteractableActors() { return InteractableActors; }

private:
	UPROPERTY()
	TArray<AInteractableActor*>	InteractableActors;
};
