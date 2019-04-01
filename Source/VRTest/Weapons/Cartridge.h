// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/InteractableActor.h"
#include "Cartridge.generated.h"

class UStaticMeshComponent;
class USoundCue;
class AMagazine;

UCLASS()
class VRTEST_API ACartridge : public AInteractableActor
{
	GENERATED_BODY()
	
public:	
	ACartridge();

	UStaticMeshComponent*	GetMesh() { return CasingMeshComponent; }

	/* */
	virtual bool CanLoadIntoMagazine(AMagazine* Magazine);

	virtual void OnEjected(bool bEmpty);

protected:
	virtual void BeginPlay() override;

	virtual void GetInteractionConditions(const AHand* InteractingHand, TArray<FInteractionHelperReturnParams>& Params) const override;

	virtual void OnDrop(AHand* Hand) override;

	virtual void Tick(float DeltaTime);

	/* Destroys self after loading */
	virtual void LoadIntoMagazine(AMagazine* Magazine);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Cartridge")
	UStaticMeshComponent* CasingMeshComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cartridge")
	UStaticMesh* CasingMeshFull;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cartridge")
	UStaticMesh* CasingMeshEmpty;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cartridge")
	USoundCue* BounceSound;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cartridge")
	USoundCue* LoadSound;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cartridge")
	TArray<TSubclassOf<AMagazine>>	CompatibleMagazines;

private:
	UPROPERTY()
	AMagazine*	LoadableMagazine;

	bool bSpentRound;
	bool bHasBounced;
};
