// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractionHelper.h"

AInteractionHelper::AInteractionHelper()
{
	bInitialized = false;
	SetActorEnableCollision(false);
}

void AInteractionHelper::SetHelperParams(FInteractionHelperReturnParams& Param)
{
	ensure(Param.AssociatedActor);

	SetActorHiddenInGame(false);
	SetActorLocation(Param.WorldLocation);

	auto OldParams = InteractionParams;

	InteractionParams = Param;

	bool bNewHelperParams = Param.Tag != OldParams.Tag || Param.AssociatedActor != OldParams.AssociatedActor;
	bool bResetEvents = !bInitialized || InteractionParams.bCanUse != OldParams.bCanUse || bNewHelperParams;

	if(bResetEvents)
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

