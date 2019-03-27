// Fill out your copyright notice in the Description page of Project Settings.

#include "VRGameState.h"
#include "Kismet/GameplayStatics.h"
#include "ExtendedCharacter.h"

AVRGameState::AVRGameState()
{
	CurrentTimeDilation = 1.0f;
	BulletTimeModifier = 0.2f;
	BulletTimeInterpSpeed = 8.0f;
	BulletTimeDuration = 3.0f;
	bBulletTime = false;

	BeginScoringTimestamp = -1.0f;

	NumDeadCharacters = 0;

	PrimaryActorTick.bCanEverTick = true;
}

void AVRGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	float TargetTimeDilation = 1.0f;
	if (bBulletTime)
	{
		TargetTimeDilation = BulletTimeModifier;
	}

	if (CurrentTimeDilation != TargetTimeDilation)
	{
		CurrentTimeDilation = FMath::FInterpTo(CurrentTimeDilation, TargetTimeDilation, DeltaSeconds, BulletTimeInterpSpeed);

		UGameplayStatics::SetGlobalPitchModulation(this, TargetTimeDilation, 1.0f);
		UGameplayStatics::SetGlobalTimeDilation(this, CurrentTimeDilation);
	}
}

void AVRGameState::AddSpawnedCharacter(AExtendedCharacter* Character)
{
	Character->OnKilledDelegate.AddDynamic(this, &ThisClass::OnCharacterKilled);

	SpawnedCharacters.Add(Character);
}

void AVRGameState::TriggerBulletTime(float Duration)
{
	UGameplayStatics::SetGlobalPitchModulation(this, BulletTimeModifier, 1.0f);
	bBulletTime = true;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_BulletTime, this, &ThisClass::FinishBulletTime, Duration * BulletTimeModifier);
}

void AVRGameState::OnCharacterKilled(AExtendedCharacter* Character, AController* Killer, const FHitResult& HitEvent)
{
	if (Killer)
	{
		const float DistanceToCharacter = FVector::Dist(Killer->GetPawn()->GetActorLocation(), Character->GetActorLocation());
		if (DistanceToCharacter < 300.0f)
		{
			TriggerBulletTime(BulletTimeDuration);
		}
	}
 	
	NumDeadCharacters++;

	// Killed the last character
	if (NumDeadCharacters == SpawnedCharacters.Num())
	{
		if (FMath::RandRange(0.0f, 1.0f) <= 0.35f)
		{
			TriggerBulletTime(BulletTimeDuration);
		}
	}

	if (HitEvent.BoneName == Character->HeadBone)
	{
		NumHeadshots++;
	}
}

void AVRGameState::FinishBulletTime()
{
	UGameplayStatics::SetGlobalPitchModulation(this, 1.0f, 1.0f);
	bBulletTime = false;
}

void AVRGameState::ResetScores()
{
	NumPlayerShotsFired = 0;
	NumHeadshots = 0;
	NumDeadCharacters = 0;
}

void AVRGameState::BeginScoring()
{
	BeginScoringTimestamp = GetWorld()->GetTimeSeconds();

	ResetScores();
}

void AVRGameState::FinishScoring()
{
	FinishScoringTimestamp = GetWorld()->GetTimeSeconds();
	
	LastMatchDuration = FinishScoringTimestamp - BeginScoringTimestamp;
}

float AVRGameState::GetTotalScore() const
{
	float TotalScore = 0;

	// Todo: transfer to member variables
	int32 PointsPerKill = 10;
	int32 PointsPerHeadshot = 5;
	float PointsDeductedPerShot = 0.1;

	TotalScore += (PointsPerKill * NumDeadCharacters);
	TotalScore += (PointsPerHeadshot * NumHeadshots);
	TotalScore -= (PointsDeductedPerShot * NumPlayerShotsFired);

	return TotalScore;
}