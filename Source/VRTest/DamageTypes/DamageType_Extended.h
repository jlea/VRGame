// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/DamageType.h"
#include "DamageType_Extended.generated.h"

/**
 * 
 */
UCLASS()
class UDamageType_Extended : public UDamageType
{
	GENERATED_BODY()

	UDamageType_Extended();

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bSeverLimbs;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bForceRagdoll;
};
