// Fill out your copyright notice in the Description page of Project Settings.

#include "PumpActionFirearm.h"
#include "DrawDebugHelpers.h"
#include "Hand.h"

APumpActionFirearm::APumpActionFirearm()
{
	bHasEjectedRound = true;
	bHasPumped = true;
}

void APumpActionFirearm::OnBeginInteraction(AHand* Hand)
{
	Super::OnBeginInteraction(Hand);

	if (Hand == InteractingHand)
	{
		// Update pump progress

		// Get position of hand between two points
		const FVector StartLocation = FirearmMesh->GetSocketLocation(PumpStartSocket);
		const FVector EndLocation = FirearmMesh->GetSocketLocation(PumpEndSocket);
		
		const FVector HandLocation = Hand->GetHandSelectionOrigin();

		const FVector ClosestPoint = FMath::ClosestPointOnLine(StartLocation, EndLocation, HandLocation);

		DrawDebugSphere(GetWorld(), ClosestPoint, 5.0f, 8, FColor::Red, false, 0.1f);
		
		const float DistanceToStart = FVector::Dist2D(ClosestPoint, StartLocation);
		const float DistanceToEnd = FVector::Dist2D(ClosestPoint, EndLocation);


		UE_LOG(LogTemp, Warning, TEXT("Distance to start and end: %g %g"), DistanceToStart, DistanceToEnd);

	}
}

void APumpActionFirearm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (InteractingHand)
	{
		// Pumping it?
		if (!bHasEjectedRound)
		{
			if (PumpProgress >= 1.0)
			{
				bHasEjectedRound = true;

				OnPumpBack();
			}
		}
		else if(bHasEjectedRound && !bHasPumped)
		{
			if (PumpProgress <= 0.0f)
			{
				bHasPumped = true;

				OnPumpForward();
			}
		}
		
	}
}

bool APumpActionFirearm::CanFire()
{
	if (!bHasPumped)
	{
		return false;
	}

	if (!bHasEjectedRound)
	{
		return false;
	}

	return Super::CanFire();
}

void APumpActionFirearm::Fire()
{
	Super::Fire();

	bHasEjectedRound = false;
	bHasPumped = false;
}
