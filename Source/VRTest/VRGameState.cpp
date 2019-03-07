// Fill out your copyright notice in the Description page of Project Settings.

#include "VRGameState.h"
#include "Kismet/GameplayStatics.h"
#include "ExtendedCharacter.h"

AVRGameState::AVRGameState()
{
	CurrentTimeDilation = 1.0f;
	BulletTimeModifier = 0.35f;
	BulletTimeInterpSpeed = 12.0f;
	bBulletTime = false;
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
		UGameplayStatics::SetGlobalTimeDilation(this, TargetTimeDilation);
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

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_BulletTime, this, &ThisClass::FinishBulletTime, Duration);
}

void AVRGameState::OnCharacterKilled(AExtendedCharacter* Character, AController* Killer, const FHitResult& HitEvent)
{
	if (HitEvent.BoneName == Character->HeadBone)
	{
		
	}
}

void AVRGameState::FinishBulletTime()
{
	UGameplayStatics::SetGlobalPitchModulation(this, 1.0f, 1.0f);
	bBulletTime = false;
}
