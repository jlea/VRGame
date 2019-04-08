// Fill out your copyright notice in the Description page of Project Settings.

#include "TeleportDestination.h"
#include "Character/PlayerPawn.h"
#include "Spawner.h"
#include "Components/SphereComponent.h"

// Sets default values
ATeleportDestination::ATeleportDestination()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>("CollisionSphere");
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionSphere->ShapeColor = FColor::White;

	bDisableOnTeleport = true;
	bStartEnabled = false;
	bHovered = false;
	FinishedSpawners = 0;

	RootComponent = CollisionSphere;
}

// Called when the game starts or when spawned
void ATeleportDestination::BeginPlay()
{
	Super::BeginPlay();

	SetDestinationEnabled(bStartEnabled);

	for (auto Spawner : LinkedSpawners)
	{
		if (!Spawner)
		{
			continue;
		}

		Spawner->OnAllPawnsKilledDelegate.AddDynamic(this, &ThisClass::OnSpawnerFinished);
	}
}

void ATeleportDestination::Reset()
{
	Super::Reset();

	SetDestinationEnabled(bStartEnabled);

	FinishedSpawners = 0;
}

// Called every frame
void ATeleportDestination::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATeleportDestination::TeleportToDestination(APlayerPawn* Pawn, AHand* Hand)
{
	check(Pawn);

	FVector TeleportDestination = GetActorLocation();

	// Now trace downwards
	FHitResult PostTraceHit;
	GetWorld()->LineTraceSingleByChannel(PostTraceHit, TeleportDestination, (TeleportDestination + FVector::UpVector * -1000.0f), ECC_Visibility);
	if (PostTraceHit.IsValidBlockingHit())
	{
		TeleportDestination = PostTraceHit.ImpactPoint;
	}

	// Hide our current location 
	auto LastTeleportDestination = Pawn->LastTeleportDestination;
	if(LastTeleportDestination)
	{
		LastTeleportDestination->bHovered = false;
		LastTeleportDestination->OnTeleportUnhovered(Pawn, Hand);
		LastTeleportDestination->OnPlayerLeft();
	}

	Pawn->SetActorLocation(TeleportDestination);
	Pawn->SetActorRotation(GetActorRotation());

	Pawn->LastTeleportDestination = this;

	Pawn->OnTeleported(TeleportDestination);

	OnTeleportedDelegate.Broadcast(this);

	OnPlayerArrived();

	if (bDisableOnTeleport)
	{
		SetDestinationEnabled(false);
	}

	// Trigger any required spawns
	for (auto Spawner : SpawnersToTriggerOnArrival)
	{
		Spawner->SpawnPawn();
	}
}

void ATeleportDestination::SetDestinationEnabled(bool bInEnabled)
{
	bTeleportEnabled = bInEnabled;

	if (bTeleportEnabled)
	{
		SetActorEnableCollision(true);
		CollisionSphere->ShapeColor = FColor::Blue;
	}
	else
	{
		SetActorEnableCollision(false);
		CollisionSphere->ShapeColor = FColor::Red;
	}

	OnTeleportStateChanged(bTeleportEnabled);
}

void ATeleportDestination::OnSpawnerFinished(ASpawner* Spawner)
{
	FinishedSpawners++;

	if (FinishedSpawners == LinkedSpawners.Num())
	{
		SetDestinationEnabled(true);
	}
}

