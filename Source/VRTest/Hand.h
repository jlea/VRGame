// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "MotionControllerComponent.h"
#include "Hand.generated.h"

class AInteractableActor;
class USphereComponent;
class APlayerController;
class APlayerPawn;

UENUM(BlueprintType)
enum class EHandGripState : uint8
{
	Open,
	CanGrab,
	Grab
};

UCLASS()
class VRTEST_API AHand : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHand();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, Category = "Hand")
	bool bWantsGrab;

	UPROPERTY(BlueprintReadOnly, Category = "Teleport")
	bool bWantsTeleport;

	UPROPERTY(BlueprintReadOnly, Category = "Teleport")
	FPredictProjectilePathResult TeleportResult;

	UPROPERTY(BlueprintReadOnly, Category = "Teleport")
	FVector ValidTeleportLocation;
	
	UPROPERTY(BlueprintReadOnly, Category = "Teleport")
	bool bHasValidTeleportLocation;

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	float TeleportProjectileVelocity;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Hand")
	USkeletalMeshComponent*	HandMesh;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Hand")
	UMotionControllerComponent*	MotionController;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Hand")
	USceneComponent*	HandOrigin;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Hand")
	USphereComponent*	SphereCollision;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void OnDropPressed();
	void OnGrabPressed();
	void OnGrabReleased();

	void OnTeleportPressed();
	void OnTeleportReleased();

	UFUNCTION(BlueprintPure, Category = "Hand")
	USphereComponent* GetSphereComponent() const { return SphereCollision; }

	UFUNCTION(BlueprintPure, Category = "Hand")
	USkeletalMeshComponent* GetHandMesh() const { return HandMesh; }

	UFUNCTION(BlueprintPure, Category = "Player")
	APlayerController*	GetPlayerController();

	UFUNCTION(BlueprintPure, Category = "Player")
	APlayerPawn*	GetPlayerPawn();

	UPROPERTY(BlueprintReadOnly, Category = "Hand")
	EControllerHand	HandType;

	UPROPERTY(BlueprintReadWrite, Category = "Hand")
	EHandGripState GripState;

	UFUNCTION(BlueprintPure, Category = "Hand")
	AInteractableActor*	GetClosestActorToHand() { return ClosestNearbyActor; }

	UFUNCTION(BlueprintPure, Category = "Hand")
	AInteractableActor*	GetInteractingActor() { return InteractingActor; }

	UFUNCTION(BlueprintPure, Category = "Hand")
	TArray<AInteractableActor*>	GetNearbyActors()
	{
		return NearbyActors;
	}

	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetHandSelectionOrigin() const;

	void ResetMeshToOrigin();

private:
	void TryTeleport();
	void UpdateTeleport();

	void UpdateNearbyActors();

	void Grab(AInteractableActor* InteractableActor);
	void ReleaseActor();

	UPROPERTY()
	AInteractableActor* ClosestNearbyActor;

	UPROPERTY()
	TArray<AInteractableActor*> NearbyActors;

	FTransform CachedMeshTransform;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Hand")
	AInteractableActor* InteractingActor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hand")
	UHapticFeedbackEffect_Base* GripEffect;

	//////////////////////////////////////////////////////////////////////////
	//	Events
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Hand")
	void OnHoverActorChanged(AInteractableActor* NewActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hand")
	void ReceiveOnReleaseHeldActor(AInteractableActor* OldHeldActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hand")
	void ReceiveOnGrab(AInteractableActor* NewActor);
};
