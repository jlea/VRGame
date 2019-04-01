// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HandEnums.generated.h"

UENUM(BlueprintType)
enum class EDirectionPadInput : uint8
{
	Forward,
	Left,
	Right,
	Back
};

UENUM(BlueprintType)
enum class EHandGripState : uint8
{
	Open,
	CanGrab,
	Grab
};

UENUM(BlueprintType)
enum class EInteractPriority : uint8
{
	Low,
	Medium,
	High
};

USTRUCT(BlueprintType)
struct FInteractionHelperReturnParams
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadOnly)
		FVector Location;

	UPROPERTY(BlueprintReadOnly)
		FString Tag;

	UPROPERTY(BlueprintReadOnly)
		bool	bRenderHelper;

	UPROPERTY(BlueprintReadOnly)
		bool	bCanUse;


	/** defaults */
	FInteractionHelperReturnParams()
	{
		bRenderHelper = true;
		bCanUse = true;
	}
};