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

UENUM(BlueprintType)
enum class EInteractionHelperState : uint8
{
	Uninitialized,
	Invalid,
	Valid,
	Active
};

USTRUCT(BlueprintType)
struct FInteractionHelperReturnParams
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	FVector WorldLocation;

	/* Used to determine actions in response to the helper */
	UPROPERTY(BlueprintReadOnly)
	FString Tag;

	/* What message should be displayed to the user when this helper is visible */
	UPROPERTY(BlueprintReadOnly)
	FString Message;

	UPROPERTY(BlueprintReadOnly)
	EInteractionHelperState HelperState;

	UPROPERTY(BlueprintReadOnly)
	bool bShouldRender;

	/** defaults */
	FInteractionHelperReturnParams()
	{
		bShouldRender = true;
		HelperState = EInteractionHelperState::Invalid;
	}
};