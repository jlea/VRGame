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
};
