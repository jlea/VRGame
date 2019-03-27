// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TeleportDestination.generated.h"

class USphereComponent;
class APlayerPawn;
class AHand;
class ASpawner;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTeleportDelegate, ATeleportDestination*, Destination);

UCLASS()
class VRTEST_API ATeleportDestination : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATeleportDestination();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Reset() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void TeleportToDestination(APlayerPawn* Pawn, AHand* Hand);

	UFUNCTION(BlueprintImplementableEvent)
	void OnTeleportHovered(APlayerPawn* Pawn, AHand* Hand);

	UFUNCTION(BlueprintImplementableEvent)
	void OnTeleportUnhovered(APlayerPawn* Pawn, AHand* Hand);

	UFUNCTION(BlueprintImplementableEvent)
	void OnTeleportStateChanged(bool bEnabled);

	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayerLeft();

	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayerArrived();

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category = "Teleport")
	bool bStartEnabled;

	UFUNCTION(BlueprintCallable, Category = "Teleport")
	void SetDestinationEnabled(bool bInEnabled);

	UFUNCTION(BlueprintPure, Category = "Teleport")
	bool IsEnabled() { return bEnabled; }

	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, Category = "Teleport")
	FTeleportDelegate OnTeleportedDelegate;

	/* Enable this destination when all these spawners have triggered. */
	UPROPERTY(EditInstanceOnly, Category = "Teleport")
	TArray<ASpawner*>	LinkedSpawners;

	/* These spawners will trigger on arrival at this location */
	UPROPERTY(EditInstanceOnly, Category = "Teleport")
	TArray<ASpawner*>	SpawnersToTriggerOnArrival;

	UFUNCTION()
	void OnSpawnerFinished(ASpawner* Spawner);

	bool bHovered;

protected:
	UPROPERTY(VisibleAnywhere)
	USphereComponent*	CollisionSphere;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category = "Teleport")
	bool bDisableOnTeleport;

private:

	int8 FinishedSpawners;

	bool bEnabled;
};
