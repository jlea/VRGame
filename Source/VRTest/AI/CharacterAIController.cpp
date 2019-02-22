// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterAIController.h"

ACharacterAIController::ACharacterAIController()
{
	TurnInterpSpeed = 5.0f;
	bAllowStrafe = true;

	bAttachToPawn = true;
}

void ACharacterAIController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn /* = true */)
{
	APawn* const MyPawn = GetPawn();
	if (MyPawn)
	{
		FRotator NewControlRotation = GetControlRotation();

		// Look toward focus
		const FVector FocalPoint = GetFocalPoint();
		if (FAISystem::IsValidLocation(FocalPoint))
		{
			NewControlRotation = (FocalPoint - MyPawn->GetPawnViewLocation()).Rotation();
		}
		else if (bSetControlRotationFromPawnOrientation)
		{
			NewControlRotation = MyPawn->GetActorRotation();
		}

		// Don't pitch view unless looking at another pawn
		if (NewControlRotation.Pitch != 0 && Cast<APawn>(GetFocusActor()) == nullptr)
		{
			NewControlRotation.Pitch = 0.f;
		}

		FRotator InterpolatedRotator = FMath::RInterpTo(GetControlRotation(), NewControlRotation, DeltaTime, TurnInterpSpeed);

		SetControlRotation(InterpolatedRotator);

		if (bUpdatePawn)
		{
			const FRotator CurrentPawnRotation = MyPawn->GetActorRotation();

			if (CurrentPawnRotation.Equals(NewControlRotation, 1e-3f) == false)
			{
				MyPawn->FaceRotation(NewControlRotation, DeltaTime);
			}
		}
	}
}
