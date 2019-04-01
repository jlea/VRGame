// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "HandEnums.h"
#include "InteractionHelperComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRTEST_API UInteractionHelperComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UInteractionHelperComponent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnInteractionEnabled();

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnInteractionDisabled();

	void SetHelperParams(FInteractionHelperReturnParams& Param);
	void SetNoHelper();
};
