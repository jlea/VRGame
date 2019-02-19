// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Firearm.h"
#include "PumpActionFirearm.generated.h"


/**
 * 
 */
UCLASS()
class VRTEST_API APumpActionFirearm : public AFirearm
{
	GENERATED_BODY()
public:
	APumpActionFirearm();

	virtual void OnBeginInteraction(AHand* Hand) override;
	virtual void Tick(float DeltaTime) override;

	virtual bool CanFire() override; 

	virtual void Fire();

	UFUNCTION(BlueprintImplementableEvent, Category = "Pump Action")
	void OnPumpBack();	
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Pump Action")
	void OnPumpForward();

	bool bHasEjectedRound;
	bool bHasPumped; 
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Pump Action")
	FName PumpStartSocket;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Pump Action")
	FName PumpEndSocket;

	UPROPERTY(BlueprintReadOnly, Category = "Pump Action")
	float PumpProgress;
};
