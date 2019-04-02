// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HandEnums.h"
#include "InteractionHelper.generated.h"


UCLASS()
class VRTEST_API AInteractionHelper : public AActor
{
	GENERATED_BODY()

public:
	AInteractionHelper();

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnInteractionEnabled();

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnInteractionDisabled();

	void SetHelperParams(FInteractionHelperReturnParams& Param);
	void SetNoHelper();

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FInteractionHelperReturnParams InteractionParams;

	bool bInitialized;
};
