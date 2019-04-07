// Fill out your copyright notice in the Description page of Project Settings.

#include "PumpActionFirearm.h"
#include "DrawDebugHelpers.h"
#include "Interactable/Hand.h"

APumpActionFirearm::APumpActionFirearm()
{
	bEjectRoundOnFire = false;
	bHasInternalMagazine = true;
	bSnapSlideForwardOnRelease = false;
	AmmoLoadType = EFirearmAmmoLoadType::PumpAction;

	SlideStartSocket = TEXT("PumpStart");
	SlideEndSocket = TEXT("PumpEnd");
	SlideAttachSocket = TEXT("PumpAttach");
}