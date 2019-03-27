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
