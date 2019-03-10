// Fill out your copyright notice in the Description page of Project Settings.

#include "VRGameMode.h"
#include "EngineUtils.h"
#include "VRGameState.h"
#include "UI/VRHUD.h"
#include "Spawner.h"
#include "PlayerPawn.h"

AVRGameMode::AVRGameMode()
{
	GameStateClass = AVRGameState::StaticClass();
	HUDClass = AVRHUD::StaticClass();

	DefaultPawnClass = APlayerPawn::StaticClass();
}

void AVRGameMode::ResetLevel()
{
	Super::ResetLevel();

	Spawners.Empty();

	for (TActorIterator<ASpawner> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		Spawners.Add(*ActorItr);
	}
}
