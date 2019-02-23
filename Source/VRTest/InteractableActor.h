// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableActor.generated.h"

class AHand;
class AHUD;

UENUM(BlueprintType)
enum class EInteractPriority : uint8
{
	Low,
	Medium,
	High
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableActorEvent, AInteractableActor*, InteractableActor);

UCLASS()
class VRTEST_API AInteractableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual bool CanGrab(const AHand* Hand);
	//virtual bool CanDrop(const AHand* Hand) { return true; }

	virtual void DrawHUD(AHUD* HUD, AHand* InteractingHand);

	void BeginGrab(AHand* Hand);
	void EndGrab(AHand* Hand);	
	void Drop(AHand* Hand);

	UFUNCTION(BlueprintPure, Category = "VR")
	AHand*	GetAttachedHand() { return AttachedHand; }
	
	UFUNCTION(BlueprintPure, Category = "VR")
	AHand*	GetInteractingHand() { return InteractingHand; }

protected:
	virtual void OnBeginPickup(AHand* Hand);
	virtual void OnBeginInteraction(AHand* Hand);
	virtual void OnDrop(AHand* Hand);
	virtual void OnEndInteraction(AHand* Hand);

	UFUNCTION(BlueprintImplementableEvent, Category = "VR")
	void ReceiveOnBeginPickup(AHand* Hand);

	UFUNCTION(BlueprintImplementableEvent, Category = "VR")
	void ReceiveOnBeginInteraction(AHand* Hand);

	UFUNCTION(BlueprintImplementableEvent, Category = "VR")
	void ReceiveOnDrop(AHand* Hand);

	UFUNCTION(BlueprintImplementableEvent, Category = "VR")
	void ReceiveOnEndInteraction(AHand* Hand);

	UPROPERTY(BlueprintReadOnly)
	AHand*	AttachedHand;

	UPROPERTY(BlueprintReadOnly)
	AHand*	InteractingHand;

public:
	UPROPERTY(EditAnywhere, Category = "Gameplay")
	bool bDropOnRelease;

	UPROPERTY(EditAnywhere, Category = "Gameplay")
	EInteractPriority	InteractPriority;

	//////////////////////////////////////////////////////////////////////////
	//	Delegates

	UPROPERTY(BlueprintAssignable, Category = "VR")
	FInteractableActorEvent OnInteractionStart;

	UPROPERTY(BlueprintAssignable, Category = "VR")
	FInteractableActorEvent OnInteractionEnd;

	UPROPERTY(BlueprintAssignable, Category = "VR")
	FInteractableActorEvent OnDropDelegate;

	UPROPERTY(BlueprintAssignable, Category = "VR")
	FInteractableActorEvent OnPickedUpDelegate;


	//////////////////////////////////////////////////////////////////////////
	//	Special

};
