// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/InteractableActor.h"
#include "Magazine.generated.h"

class AFirearm;
class USoundCue;

UCLASS()
class VRTEST_API AMagazine : public AInteractableActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagazine();

	virtual void BeginPlay() override;

	virtual bool CanHolster() const override; 
	virtual void GetInteractionConditions(const AHand* InteractingHand, TArray<FInteractionHelperReturnParams>& Params) const override;
	virtual FText GetDefaultInteractionText() const;

	virtual void OnBeginPickup(AHand* Hand) override;
	virtual void OnDrop(AHand* Hand) override;

	void OnMagazineLoaded(AFirearm*	Firearm);
	void OnMagazineEjected(AFirearm* Firearm);

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Magazine")
	void ReplenishMagazine();

	UFUNCTION(BlueprintPure, Category = "Magazine")
	bool IsReadyToLoadCartridge(ACartridge* Cartridge);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Magazine")
	UStaticMeshComponent* MagazineMesh;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Magazine")
	int32 AmmoCount;

	UPROPERTY(BlueprintReadOnly, Category = "Magazine")
	int32 CurrentAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Magazine")
	TArray<TSubclassOf<AFirearm>>	CompatibleFirearms;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Magazine")
	USoundCue* LoadSound;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Magazine")
	USoundCue* EjectSound;

	UPROPERTY()
	AFirearm*	AttachedFirearm;

	UPROPERTY()
	AFirearm*	LoadableFirearm;

	bool bInteractable;
};
