// Fill out your copyright notice in the Description page of Project Settings.

#include "TeleportDestination.h"
#include "PlayerPawn.h"
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
	bStartEnabled = true;

	RootComponent = CollisionSphere;
}

// Called when the game starts or when spawned
void ATeleportDestination::BeginPlay()
{
	Super::BeginPlay();

	SetDestinationEnabled(bStartEnabled);
}

void ATeleportDestination::Reset()
{
	Super::Reset();

	SetDestinationEnabled(bStartEnabled);
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

	Pawn->SetActorLocation(TeleportDestination);
	Pawn->SetActorRotation(GetActorRotation());

	Pawn->OnTeleported(TeleportDestination);

	OnTeleportedDelegate.Broadcast(this);

	if (bDisableOnTeleport)
	{
		SetDestinationEnabled(false);
	}
}

void ATeleportDestination::SetDestinationEnabled(bool bInEnabled)
{
	bEnabled = bInEnabled;

	UE_LOG(LogTemp, Warning, TEXT("%d"), bEnabled);
	
	if (bEnabled)
	{
		SetActorEnableCollision(true);
		CollisionSphere->ShapeColor = FColor::Blue;
	}
	else
	{
		SetActorEnableCollision(false);
		CollisionSphere->ShapeColor = FColor::Red;
	}
}

