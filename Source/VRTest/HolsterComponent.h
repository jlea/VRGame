// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "HolsterComponent.generated.h"

class AInteractableActor;

/**
 * 
 */ 
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VRTEST_API UHolsterComponent : public UBoxComponent
{
	GENERATED_BODY()

	UHolsterComponent();

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

private:
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void HolsterActor(AInteractableActor* ActorToHolster);

public:
	UPROPERTY(BlueprintReadOnly)
	TArray<AInteractableActor*>	HolsteredActors;

	UPROPERTY(BlueprintReadOnly)
	TArray<AInteractableActor*>	ValidHoveredActors;
};
