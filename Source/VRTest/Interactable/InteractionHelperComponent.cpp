// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractionHelperComponent.h"

UInteractionHelperComponent::UInteractionHelperComponent()
{
}

void UInteractionHelperComponent::SetHelperParams(FInteractionHelperReturnParams& Param)
{
	SetVisibility(true, true);
	SetWorldLocation(Param.Location);

	// Compare to cached state
	if (Param.bCanUse)
	{
		OnInteractionEnabled();
	}
	else
	{
		OnInteractionDisabled();
	}
}

void UInteractionHelperComponent::SetNoHelper()
{
	// Hide this helper from the game view 
	SetVisibility(false, true);
}

