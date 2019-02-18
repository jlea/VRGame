// Fill out your copyright notice in the Description page of Project Settings.

#include "VRGameMode.h"
#include "VRGameState.h"
#include "UI/VRHUD.h"
#include "PlayerPawn.h"

AVRGameMode::AVRGameMode()
{
	GameStateClass = AVRGameState::StaticClass();
	HUDClass = AVRHUD::StaticClass();

	DefaultPawnClass = APlayerPawn::StaticClass();
}
