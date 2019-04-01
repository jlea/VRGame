// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterAIController.h"
#include "Perception/PawnSensingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Character/ExtendedCharacter.h"
#include "Character/PlayerPawn.h"
#include "NetworkingDistanceConstants.h"
#include "Weapons/Firearm.h"

ACharacterAIController::ACharacterAIController()
{
	TurnInterpSpeed = 5.0f;
	bAllowStrafe = true;
	bAttachToPawn = true;

	TargetEnemyKeyName = "TargetEnemy";
	TargetAliveKeyName = "TargetAlive";
	TargetHasLOSKeyName = "TargetHasLOS";
	TargetLastSeenPositionKeyName = "TargetLastSeenPosition";
	TargetLastSeenVelocityKeyName = "TargetLastSeenVelocity";
	TargetLastSeenTimeKeyName = "TargetLastSeenTime";
	TargetDistanceKeyName = "TargetDistance";

	Suppression = 0.0f;

	bShouldFire = false;
}

bool ACharacterAIController::LineOfSightTo(const AActor* Other, FVector ViewPoint, bool bAlternateChecks) const
{
	if (Other == nullptr)
	{
		return false;
	}

	if (ViewPoint.IsZero())
	{
		FRotator ViewRotation;
		GetActorEyesViewPoint(ViewPoint, ViewRotation);

		// if we still don't have a view point we simply fail
		if (ViewPoint.IsZero())
		{
			return false;
		}
	}

	FVector TargetLocation = Other->GetTargetLocation(GetPawn());

	FCollisionQueryParams CollisionParams(SCENE_QUERY_STAT(LineOfSight), true, this->GetPawn());
	CollisionParams.AddIgnoredActor(Other);

	bool bHit = GetWorld()->LineTraceTestByChannel(ViewPoint, TargetLocation, ECC_Visibility, CollisionParams);
	if (!bHit)
	{
		return true;
	}

	// if other isn't using a cylinder for collision and isn't a Pawn (which already requires an accurate cylinder for AI)
	// then don't go any further as it likely will not be tracing to the correct location
	const APawn * OtherPawn = Cast<const APawn>(Other);
	if (!OtherPawn && Cast<UCapsuleComponent>(Other->GetRootComponent()) == NULL)
	{
		return false;
	}

	const FVector OtherActorLocation = Other->GetActorLocation();
	const float DistSq = (OtherActorLocation - ViewPoint).SizeSquared();
	if (DistSq > FARSIGHTTHRESHOLDSQUARED)
	{
		return false;
	}

	if (!OtherPawn && (DistSq > NEARSIGHTTHRESHOLDSQUARED))
	{
		return false;
	}

	float OtherRadius, OtherHeight;
	Other->GetSimpleCollisionCylinder(OtherRadius, OtherHeight);

	if (!bAlternateChecks || !bLOSflag)
	{
		//try viewpoint to head
		FHitResult OutHit;
		bHit = GetWorld()->LineTraceSingleByChannel(OutHit, ViewPoint, OtherActorLocation + FVector(0.f, 0.f, OtherHeight), ECC_Visibility, CollisionParams);
		
		if (!bHit)
		{
			return true;
		}
	}

	if (!bSkipExtraLOSChecks && (!bAlternateChecks || bLOSflag))
	{
		// only check sides if width of other is significant compared to distance
		if (OtherRadius * OtherRadius / (OtherActorLocation - ViewPoint).SizeSquared() < 0.0001f)
		{
			return false;
		}
		//try checking sides - look at dist to four side points, and cull furthest and closest
		FVector Points[4];
		Points[0] = OtherActorLocation - FVector(OtherRadius, -1 * OtherRadius, 0);
		Points[1] = OtherActorLocation + FVector(OtherRadius, OtherRadius, 0);
		Points[2] = OtherActorLocation - FVector(OtherRadius, OtherRadius, 0);
		Points[3] = OtherActorLocation + FVector(OtherRadius, -1 * OtherRadius, 0);
		int32 IndexMin = 0;
		int32 IndexMax = 0;
		float CurrentMax = (Points[0] - ViewPoint).SizeSquared();
		float CurrentMin = CurrentMax;
		for (int32 PointIndex = 1; PointIndex<4; PointIndex++)
		{
			const float NextSize = (Points[PointIndex] - ViewPoint).SizeSquared();
			if (NextSize > CurrentMin)
			{
				CurrentMin = NextSize;
				IndexMax = PointIndex;
			}
			else if (NextSize < CurrentMax)
			{
				CurrentMax = NextSize;
				IndexMin = PointIndex;
			}
		}

		for (int32 PointIndex = 0; PointIndex<4; PointIndex++)
		{
			if ((PointIndex != IndexMin) && (PointIndex != IndexMax))
			{
				bHit = GetWorld()->LineTraceTestByChannel(ViewPoint, Points[PointIndex], ECC_Visibility, CollisionParams);
				if (!bHit)
				{
					return true;
				}
			}
		}
	}
	return false;
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

		//Que up our update
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_UpdateTimer, this, &ThisClass::DoVisibilityCheck, 0.2f, true);

		Me->GetSensingComponent()->Activate();
		Me->GetSensingComponent()->SetSensingUpdatesEnabled(true);
		Me->GetSensingComponent()->OnHearNoise.AddDynamic(this, &ThisClass::OnHearNoise);
		Me->GetSensingComponent()->OnSeePawn.AddDynamic(this, &ThisClass::OnSeePawn);
	}
}

void ACharacterAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!GetPawn())
	{
		return;
	}

	auto Me = Cast<AExtendedCharacter>(GetPawn());
	if (Me)
	{
		auto Firearm = Me->EquippedFirearm;
		if (Firearm)
		{
			FVector DirToTarget = (GetFocalPoint() - GetPawn()->GetActorLocation()).GetSafeNormal2D();
			const float DotToTarget = FVector::DotProduct(Firearm->GetMuzzleDirection(), DirToTarget);

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
			else if (Firearm->AmmoLoadType == EFirearmAmmoLoadType::Automatic)
			{
				const bool bCanFire = DotToTarget > 0.9f && !Me->bPlayingDamageAnimation;
				if (bShouldFire && bCanFire)
				{
					Firearm->bTriggerDown = true;
				}
				else
				{
					Firearm->bTriggerDown = false;
				}
			}
		}
	}

	// Decay suppression
	float SuppressionDecrement = 10.0f;
	Suppression -= (SuppressionDecrement * DeltaTime);
	Suppression = FMath::Clamp(Suppression, 0.0f, 100.0f);
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

FVector ACharacterAIController::GetFocalPointOnActor(const AActor *Actor) const
{
	auto Pawn = Cast<APawn>(Actor);
	if (Pawn)
	{
		return Pawn->GetTargetLocation();
	}

	return Super::GetFocalPointOnActor(Actor);
}

void ACharacterAIController::SetWantsFire(bool bEnabled)
{
	bShouldFire = bEnabled;
}

void ACharacterAIController::UpdateMemory()
{
	if (!GetPawn())
	{
		return;
	}

	if (GetTargetPawn())
	{
		//If we have LOS update their last known positions
		if (TargetMemory.bHasLOS)
		{
			TargetMemory.LastSeenPosition = GetTargetPawn()->GetActorLocation();
			TargetMemory.LastSeenVelocity = GetTargetPawn()->GetVelocity();
			TargetMemory.LastSeenTimestamp = GetWorld()->GetTimeSeconds();
		}

		//Check if they're still alive
		if (TargetMemory.bIsAlive)
		{
			auto TargetCharacter = Cast<AExtendedCharacter>(GetTargetPawn());
			auto TargetPlayer = Cast<AExtendedCharacter>(GetTargetPawn());

			if (TargetCharacter)
			{
				if (TargetCharacter->bDead)
				{
					//They're dead
					OnTargetKilled();

					SetNewEnemy(nullptr);
				}
			}
			
			if (TargetPlayer)
			{
				if (TargetPlayer->bDead)
				{
					//They're dead
					OnTargetKilled();

					SetNewEnemy(nullptr);
				}
			}
		}
	}

	//Update our blackboard keys always
 	UpdateTargetBlackboard();
// 	UpdateSoundBlackboard();
}

void ACharacterAIController::UpdateTargetBlackboard()
{
	auto BlackboardComp = GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return;
	}

	float DistanceToTarget = -1.0f;

	if (TargetMemory.bHasLOS)
	{
		DistanceToTarget = (GetPawn()->GetActorLocation() - GetTargetPawn()->GetActorLocation()).Size();
	}
	else
	{
		//Use their last seen position otherwise 
		DistanceToTarget = (GetPawn()->GetActorLocation() - TargetMemory.LastSeenPosition).Size();
	}

	BlackboardComp->SetValueAsObject(TargetEnemyKeyName, TargetMemory.TargetPawn);
	BlackboardComp->SetValueAsBool(TargetHasLOSKeyName, TargetMemory.bHasLOS);
	BlackboardComp->SetValueAsBool(TargetAliveKeyName, TargetMemory.bIsAlive);
	BlackboardComp->SetValueAsVector(TargetLastSeenPositionKeyName, TargetMemory.LastSeenPosition);
	BlackboardComp->SetValueAsVector(TargetLastSeenVelocityKeyName, TargetMemory.LastSeenVelocity);
	BlackboardComp->SetValueAsFloat(TargetLastSeenTimeKeyName, GetWorld()->GetTimeSeconds() - TargetMemory.LastSeenTimestamp);
	BlackboardComp->SetValueAsFloat(TargetDistanceKeyName, DistanceToTarget);
}

void ACharacterAIController::DoVisibilityCheck()
{
	if (GetTargetPawn())
	{
		FVector ViewPoint = FVector(ForceInit);
		auto Me = Cast<AExtendedCharacter>(GetPawn());
		if (Me)
		{
			auto Firearm = Me->EquippedFirearm;
			if (Firearm)
			{
				FTransform MuzzleTransform = Firearm->FirearmMesh->GetSocketTransform(Firearm->MuzzleBone);
				ViewPoint = MuzzleTransform.GetLocation();
			}
		}

		if (LineOfSightTo(GetTargetPawn()))
		{
			SetLOSToTarget(true);
		}
		else
		{
			SetLOSToTarget(false);
		}
	}

	UpdateMemory();
}

bool ACharacterAIController::ShouldSetAsEnemy(APawn* Enemy)
{
	if (Enemy == GetTargetPawn())
	{
		return false;
	}

	auto TargetCharacter = Cast<AExtendedCharacter>(Enemy);
	if (TargetCharacter)
	{
		if (TargetCharacter->TeamId == GetGenericTeamId())
		{
			return false;
		}

		//No already dead pawns
		if (TargetCharacter->bDead)
		{
			return false;
		}

		//Not if we already have a target
		if (GetTargetPawn())
		{
			if (GetPriorityForTarget(Enemy) > GetPriorityForTarget(GetTargetPawn()))
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	auto TargetPlayer = Cast<APlayerPawn>(Enemy);
	if (TargetPlayer)
	{	
		//No already dead pawns
		if (TargetPlayer->bDead)
		{
			return false;
		}
	}

	return true;
}

void ACharacterAIController::SetNewEnemy(APawn* NewEnemy)
{
	//Set our new target
	FAITargetMemory NewMemory;
	NewMemory.TargetPawn = NewEnemy;

	if (NewEnemy)
	{
		NewMemory.bIsAlive = true;
		NewMemory.LastSeenPosition = NewEnemy->GetActorLocation();
		NewMemory.LastSeenVelocity = NewEnemy->GetVelocity();
		NewMemory.LastSeenTimestamp = GetWorld()->GetTimeSeconds();
	}

	TargetMemory = NewMemory;

	//Force a refresh
	DoVisibilityCheck();
}

void ACharacterAIController::SetLOSToTarget(bool bHasLOS)
{
	if (bHasLOS)
	{
		if (!TargetMemory.bHasLOS)
		{
			TargetMemory.bHasLOS = true;

			OnGainedLOS();
		}
	}
	else
	{
		if (TargetMemory.bHasLOS)
		{
			TargetMemory.bHasLOS = false;

			OnLostLOS();
		}
	}
}

int32 ACharacterAIController::GetPriorityForTarget(APawn* Target)
{
	//Closer = higher priority
	float Priority = 0.0f;

	//Current target gets love
	//if (Target == TargetMemory.TargetPawn)
	//	Priority += 1.0f;

	float DistanceToTarget = (GetPawn()->GetActorLocation() - Target->GetActorLocation()).Size();
	//Divide it by distance. So 10m reduces by 1.
	DistanceToTarget /= 100.0f;

	Priority -= DistanceToTarget;

	return Priority;
}

void ACharacterAIController::OnHearNoise(APawn *OtherActor, const FVector &Location, float Volume)
{
	auto HeardPawn = Cast<APawn>(OtherActor);
	if (!HeardPawn)
	{
		return;
	}

// 	if (!TargetMemory.TargetPawn || !TargetMemory.bIsAlive)
// 	{
// 		SetFocalPoint(Location);
// 	}

	if (ShouldSetAsEnemy(HeardPawn))
	{
		SetNewEnemy(HeardPawn);
	}

// 	AFPSCharacter* CharacterEmitter = Cast<AFPSCharacter>(OtherActor);
// 	if (IsTeamMate(CharacterEmitter))
// 	{
// 		return;
// 	}
// 
// 	//Update our sound memory
// 	FAISoundMemory NewSoundMemory;
// 	NewSoundMemory.bActive = true;
// 	NewSoundMemory.Timestamp = GetWorld()->GetTimeSeconds();
// 	NewSoundMemory.Location = Location;
// 	NewSoundMemory.EmitterPawn = OtherActor;
// 	NewSoundMemory.Volume = Volume;
// 
// 	SoundMemory = NewSoundMemory;
// 
// 	UpdateSoundBlackboard();
}

void ACharacterAIController::OnSeePawn(APawn *OtherPawn)
{
	if (OtherPawn == GetPawn())
	{
		return;
	}

	if (ShouldSetAsEnemy(OtherPawn))
	{
		SetNewEnemy(OtherPawn);
	}
}
