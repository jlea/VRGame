// Fill out your copyright notice in the Description page of Project Settings.

#include "PumpActionFirearm.h"
#include "DrawDebugHelpers.h"
#include "Hand.h"

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

void APumpActionFirearm::OnBeginInteraction(AHand* Hand)
{
	Super::OnBeginInteraction(Hand);
}

void APumpActionFirearm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool APumpActionFirearm::CanFire()
{
	return Super::CanFire();
}

void APumpActionFirearm::Fire()
{
	Super::Fire();
}
