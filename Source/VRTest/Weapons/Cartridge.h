// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cartridge.generated.h"

class UStaticMeshComponent;
class USoundCue;

UCLASS()
class VRTEST_API ACartridge : public AActor
{
	GENERATED_BODY()
	
public:	
	ACartridge();

	UStaticMeshComponent*	GetMesh() { return CasingMesh; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Cartridge")
	UStaticMeshComponent* CasingMesh;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cartridge")
	USoundCue* BounceSound;

	bool bEmpty;
};
