// Fill out your copyright notice in the Description page of Project Settings.

#include "VRGameMode.h"
#include "EngineUtils.h"
#include "VRGameState.h"
#include "UI/VRHUD.h"
#include "Spawner.h"
#include "GameFramework/GameSession.h"
#include "PlayerPawn.h"

AVRGameMode::AVRGameMode()
{
	GameStateClass = AVRGameState::StaticClass();
	HUDClass = AVRHUD::StaticClass();

	DefaultPawnClass = APlayerPawn::StaticClass();
}

void AVRGameMode::RestartGame()
{
	if (GameSession->CanRestartGame())
	{
		GetWorld()->ServerTravel("?Restart", false);
	}
}

void AVRGameMode::ResetLevel()
{
	Super::ResetLevel();
}

void AVRGameMode::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<ASpawner> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ActorItr->OnAllPawnsKilledDelegate.AddDynamic(this, &ThisClass::OnAllPawnsKilledForSpawner);
		ActorItr->OnPawnKilledDelegate.AddDynamic(this, &ThisClass::OnPawnKilledForSpawner);

		Spawners.Add(*ActorItr);
	}

	for (TActorIterator<APlayerPawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ActorItr->OnKilledDelegate.AddDynamic(this, &ThisClass::OnPlayerKilled);
	}
}

void AVRGameMode::OnPlayerKilled(APlayerPawn* Player, AController* Killer, const FHitResult& HitEvent)
{
	FTimerHandle TimerHandle_LevelReset;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_LevelReset, this, &ThisClass::RestartGame, 7.0f);
}

void AVRGameMode::OnAllPawnsKilledForSpawner(ASpawner* Spawner)
{

}

void AVRGameMode::OnPawnKilledForSpawner(ASpawner* Spawner, AExtendedCharacter* Pawn, AController* Killer, const FHitResult& HitResult)
{

}
