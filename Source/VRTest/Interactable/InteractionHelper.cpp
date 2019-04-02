// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractionHelper.h"

AInteractionHelper::AInteractionHelper()
{
	bInitialized = false;
	SetActorEnableCollision(false);
}

void AInteractionHelper::SetHelperParams(FInteractionHelperReturnParams& Param)
{
	SetActorHiddenInGame(false);
	SetActorLocation(Param.Location);

	auto OldParams = InteractionParams;

	InteractionParams = Param;

	if (InteractionParams.bCanUse != OldParams.bCanUse || !bInitialized)
	{
		bInitialized = true;

		// State changed
		if (Param.bCanUse)
		{
			OnInteractionEnabled();
		}
		else
		{
			OnInteractionDisabled();
		}
	}
}

void AInteractionHelper::SetNoHelper()
{
	InteractionParams = FInteractionHelperReturnParams();

	// Hide this helper from the game view 
	SetActorHiddenInGame(true);

	bInitialized = false;
}

