// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "VRHUD.generated.h"

/**
 * 
 */
UCLASS()
class VRTEST_API AVRHUD : public AHUD
{
	GENERATED_BODY()

	virtual void DrawHUD() override;
};
