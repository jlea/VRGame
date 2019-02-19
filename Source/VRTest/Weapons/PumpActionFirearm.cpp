// Fill out your copyright notice in the Description page of Project Settings.

#include "PumpActionFirearm.h"
#include "DrawDebugHelpers.h"
#include "Hand.h"

APumpActionFirearm::APumpActionFirearm()
{
	bHasPumped = false;
	bEjectRoundOnFire = false;
	bHasInternalMagazine = true;

	PumpStartSocket = TEXT("PumpStart");
	PumpEndSocket = TEXT("PumpEnd");

	AmmoLoadType = EFirearmAmmoLoadType::PumpAction;
}

void APumpActionFirearm::OnBeginInteraction(AHand* Hand)
{
	Super::OnBeginInteraction(Hand);
}

void APumpActionFirearm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (InteractingHand && InteractingHand != AttachedHand)
	{
		const FVector StartLocation = FirearmMesh->GetSocketLocation(PumpStartSocket);
		const FVector EndLocation = FirearmMesh->GetSocketLocation(PumpEndSocket);
		const FVector HandLocation = InteractingHand->GetHandSelectionOrigin();
		const FVector ClosestPoint = FMath::ClosestPointOnLine(StartLocation, EndLocation, HandLocation);

		//DrawDebugSphere(GetWorld(), ClosestPoint, 5.0f, 8, FColor::Red, false, 0.1f);

		const float DistanceToStart = FVector::Dist2D(ClosestPoint, StartLocation);
		const float DistanceToEnd = FVector::Dist2D(ClosestPoint, EndLocation);

		const float DistanceBetweenSockets = FVector::Dist2D(StartLocation, EndLocation);
		const float Ratio = 1.0f - (DistanceToStart / DistanceBetweenSockets);

		PumpProgress = Ratio;

		// Pumping it?
		if (!bHasPumped)
		{
			if (PumpProgress >= 1.0)
			{
				EjectRound();
				OnPumpBack();

				bHasPumped = true;
			}
		}
		else
		{
			if (PumpProgress <= 0.0f)
			{
				bHasPumped = false;

				LoadRoundFromMagazine();
				OnPumpForward();
			}
		}
		
	}
}

bool APumpActionFirearm::CanFire()
{
	return Super::CanFire();
}

void APumpActionFirearm::Fire()
{
	Super::Fire();
}
