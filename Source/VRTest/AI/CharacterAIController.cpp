// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterAIController.h"
#include "ExtendedCharacter.h"
#include "Firearm.h"

ACharacterAIController::ACharacterAIController()
{
	TurnInterpSpeed = 5.0f;
	bAllowStrafe = true;
	bAttachToPawn = true;

	bShouldFire = false;
}

void ACharacterAIController::UnPossess()
{
	Super::UnPossess();

	Destroy();
}

void ACharacterAIController::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);

	auto Me = Cast<AExtendedCharacter>(InPawn);
	if (Me)
	{
		SetGenericTeamId(FGenericTeamId(Me->TeamId));
	}
}

void ACharacterAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!GetPawn())
	{
		return;
	}

	FVector DirToTarget = (GetFocalPoint() - GetPawn()->GetActorLocation()).GetSafeNormal2D();
	const float DotToTarget = FVector::DotProduct(GetControlRotation().Vector(), DirToTarget);

	auto Me = Cast<AExtendedCharacter>(GetPawn());
	if (Me)
	{
		auto Firearm = Me->EquippedFirearm;
		if (Firearm)
		{
			if (Firearm->AmmoLoadType == EFirearmAmmoLoadType::SemiAutomatic)
			{
				const bool bCanFire = DotToTarget > 0.9f && !Me->bPlayingDamageAnimation;
				if (bShouldFire && bCanFire)
				{
					if (Firearm->bTriggerDown)
					{
						Firearm->bTriggerDown = false;
					}
					else
					{
						Firearm->bTriggerDown = true;
					}
				}
				else
				{
					Firearm->bTriggerDown = false;
				}
			}
		}
	}
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

void ACharacterAIController::SetWantsFire(bool bEnabled)
{
	bShouldFire = bEnabled;
}
