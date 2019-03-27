// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "HolsterComponent.generated.h"

class AInteractableActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHolsterEventDelegate);

/**
 * 
 */ 
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VRTEST_API UHolsterComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

	UHolsterComponent();

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

private:
	void HolsterActor(AInteractableActor* ActorToHolster);

public:
	UPROPERTY(BlueprintAssignable)
	FHolsterEventDelegate HolsterStateChanged;

	UPROPERTY(BlueprintReadOnly)
	TArray<AInteractableActor*>	ValidHoveredActors;

	int32 LastValidActors;
};
