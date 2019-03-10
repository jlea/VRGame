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
	NumDeadCharacters = 0;
	bBulletTime = false;

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
	if (false)
	{
		if (HitEvent.BoneName == Character->HeadBone)
		{
			TriggerBulletTime(BulletTimeDuration);
		}
	}
 	
	NumDeadCharacters++;

	// Killed the last character
	if (NumDeadCharacters == SpawnedCharacters.Num())
	{
		TriggerBulletTime(BulletTimeDuration);
	}
}

void AVRGameState::FinishBulletTime()
{
	UGameplayStatics::SetGlobalPitchModulation(this, 1.0f, 1.0f);
	bBulletTime = false;
}
