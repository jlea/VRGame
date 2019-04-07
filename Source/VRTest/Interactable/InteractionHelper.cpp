// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractionHelper.h"

AInteractionHelper::AInteractionHelper()
{
	SetActorEnableCollision(false);
}

void AInteractionHelper::SetHelperParams(FInteractionHelperReturnParams& Param)
{
	auto OldParams = InteractionParams;
	const bool bUseStateChanged = Param.HelperState != OldParams.HelperState;

	SetActorLocation(Param.WorldLocation);

	if (Param.bShouldRender)
	{
		SetActorHiddenInGame(false);
	}
	else
	{
		SetActorHiddenInGame(true);
	}

	InteractionParams = Param;

	if (bUseStateChanged)
	{
		OnInteractionStateChanged();
	}
}

void AInteractionHelper::SetHidden()
{
	FInteractionHelperReturnParams Params;
	Params.bShouldRender = false;
	Params.HelperState = EInteractionHelperState::Uninitialized;

	SetHelperParams(Params);
}