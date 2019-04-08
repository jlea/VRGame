// Fill out your copyright notice in the Description page of Project Settings.

#include "VRGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Character/ExtendedCharacter.h"

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
	Character->OnDamagedDelegate.AddDynamic(this, &ThisClass::OnCharacterDamaged);

	SpawnedCharacters.Add(Character);
}

void AVRGameState::TriggerBulletTime(float Duration)
{
	if (!bBulletTime)
	{
		OnBulletTimeBegin();
	}

	UGameplayStatics::SetGlobalPitchModulation(this, BulletTimeModifier, 1.0f);
	bBulletTime = true;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_BulletTime, this, &ThisClass::FinishBulletTime, Duration * BulletTimeModifier);
}

void AVRGameState::OnCharacterKilled(AExtendedCharacter* Character, AController* Killer, const FHitResult& HitEvent)
{
	NumDeadCharacters++;
	
	if (Killer && Killer->IsPlayerController())
	{
		bool bShouldBulletTime = Character->bTriggerBulletTimeOnKilled;

		const float DistanceToCharacter = FVector::Dist(Killer->GetPawn()->GetActorLocation(), Character->GetActorLocation());
		if (DistanceToCharacter < 300.0f)
		{
			bShouldBulletTime = true;
		}

		if (bShouldBulletTime)
		{
			TriggerBulletTime(BulletTimeDuration);
		}
	}

	if (HitEvent.BoneName == Character->HeadBone)
	{
		NumHeadshots++;
	}
}

void AVRGameState::OnCharacterDamaged(AExtendedCharacter* Character, AController* Damager, const FHitResult& HitEvent)
{
	if (Damager && Damager->IsLocalPlayerController())
	{
		NumPlayerShotsHit++;

		bool bShouldBulletTime = Character->bTriggerBulletTimeOnDamage;

		// Shot the last character in close range
		const float DistanceToCharacter = FVector::Dist(Damager->GetPawn()->GetActorLocation(), Character->GetActorLocation());
		if (DistanceToCharacter < 1000.0f && NumDeadCharacters == (SpawnedCharacters.Num() - 1))
		{
			bShouldBulletTime = true;
		}

		if (bShouldBulletTime)
		{
			TriggerBulletTime(BulletTimeDuration);
		}
	}
}

void AVRGameState::FinishBulletTime()
{
	UGameplayStatics::SetGlobalPitchModulation(this, 1.0f, 1.0f);
	bBulletTime = false;

	OnBulletTimeFinish();
}

void AVRGameState::ResetScores()
{
	NumShotsPerWeapon.Empty();
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

	int32 TotalShots = 0;
	for (auto Weapon : NumShotsPerWeapon)
	{
		TotalShots += Weapon.Value;
	}

	TotalScore -= (PointsDeductedPerShot * TotalShots);

	return TotalScore;
}