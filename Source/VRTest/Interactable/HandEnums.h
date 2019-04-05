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
	FVector WorldLocation;

	/* Used to determine actions in response to the helper */
	UPROPERTY(BlueprintReadOnly)
	FString Tag;

	/* what should be displayed to the user when this helper is visible */
	UPROPERTY(BlueprintReadOnly)
	FString Message;

	UPROPERTY(BlueprintReadOnly)
	bool	bRenderHelper;

	UPROPERTY(BlueprintReadOnly)
	bool	bCanUse;

	UPROPERTY()
	AActor* AssociatedActor;

	/** defaults */
	FInteractionHelperReturnParams()
	{
		bRenderHelper = true;
		bCanUse = true;
		AssociatedActor = nullptr;
	}
};