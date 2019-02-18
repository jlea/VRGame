// Fill out your copyright notice in the Description page of Project Settings.

#include "Hand.h"
#include "MotionControllerComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "PlayerPawn.h"
#include "DrawDebugHelpers.h"
#include "VRGameState.h"
#include "Engine.h"
#include "InteractableActor.h"

// Sets default values
AHand::AHand()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HandOrigin = CreateDefaultSubobject<USceneComponent>("Hand");
	RootComponent = HandOrigin;

	MotionController = CreateDefaultSubobject<UMotionControllerComponent>("MotionController");
	MotionController->SetupAttachment(HandOrigin);

	SphereCollision = CreateDefaultSubobject<USphereComponent>("SphereCollision");
	SphereCollision->SetupAttachment(MotionController);

	HandMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HandMesh");
	HandMesh->SetupAttachment(SphereCollision);

	bWantsGrab = false;

	SelectionOriginSocket = TEXT("SelectionOrigin");
}

// Called when the game starts or when spawned
void AHand::BeginPlay()
{
	Super::BeginPlay();

	MotionController->SetTrackingSource(HandType);

	if(HandType == EControllerHand::Left)
	{
		HandMesh->SetWorldScale3D(FVector(1.0f, 1.0f, -1.0f));
	}
}

APlayerController* AHand::GetPlayerController()
{
	auto PawnOwner = Cast<APawn>(GetOwner());
	if (PawnOwner)
	{
		auto PlayerController = Cast<APlayerController>(PawnOwner->GetController());
		if (PlayerController)
		{
			return PlayerController;
		}
	}

	return nullptr;
}

APlayerPawn* AHand::GetPlayerPawn()
{
	auto PawnOwner = Cast<APlayerPawn>(GetOwner());
	if (PawnOwner)
	{
		return PawnOwner;
	}

	return nullptr;
}

// Called every frame
void AHand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateNearbyActors();

	if (InteractingActor || bWantsGrab)
	{
		GripState = EHandGripState::Grab;
	}
	else
	{
		if (ClosestNearbyActor)
		{
			GripState = EHandGripState::CanGrab;
		}
		else
		{
			GripState = EHandGripState::Open;
		}
	}
}

void AHand::OnDropPressed()
{
	if (InteractingActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: dropping held actor %s"), *GetName(), *InteractingActor->GetName());

		InteractingActor->Drop(this);
		InteractingActor = nullptr;
	}
}

void AHand::OnGrabPressed()
{
	bWantsGrab = true;

	//GEngine->AddOnScreenDebugMessage(0, 2.0f, FColor::Yellow, FString::Printf(TEXT("%s: Grab input pressed"), *GetName()));

	if (InteractingActor)
	{
		Grab(InteractingActor);
	}
	else
	{
		AInteractableActor*	NearbyActor = GetClosestActorToHand();
		if (NearbyActor)
		{
			Grab(NearbyActor);
		}
	}
	
}

void AHand::OnGrabReleased()
{
	bWantsGrab = false;

	//GEngine->AddOnScreenDebugMessage(0, 2.0f, FColor::Yellow, FString::Printf(TEXT("%s: Grab input released"), *GetName()));

	if (InteractingActor)
	{
		ReleaseActor();
	}
}

void AHand::UpdateNearbyActors()
{
	auto VRGameState = Cast<AVRGameState>(GetWorld()->GetGameState());
	if (!VRGameState)
	{
		return;
	}

	NearbyActors.Empty();

	const float SearchDistance = SphereCollision->GetScaledSphereRadius();
	const FVector HandLocation = GetHandSelectionOrigin();

	float ClosestActorDistance = FLT_MAX;
	AInteractableActor* ClosestActor = nullptr;
	
	for (auto Actor : VRGameState->GetInteractableActors())
	{
		if (!Actor)
		{
			continue;
		}

		const float DistanceToActor = FVector::Dist(Actor->GetActorLocation(), HandLocation);
		if (DistanceToActor > (SearchDistance * 3))
		{
			continue;
		}

		if (!Actor->CanGrab(this))
		{
			continue;
		}

		if (!SphereCollision->IsOverlappingActor(Actor))
		{
			continue;
		}

		NearbyActors.Add(Actor);

		if (DistanceToActor < ClosestActorDistance)
		{
			ClosestActor = Actor;
			ClosestActorDistance = ClosestActorDistance;
		}
	}

	ClosestNearbyActor = ClosestActor;
}

FVector AHand::GetHandSelectionOrigin() const
{
	return HandMesh->GetSocketLocation(SelectionOriginSocket);
}

void AHand::Grab(AInteractableActor* InteractableActor)
{
	UE_LOG(LogTemp, Warning, TEXT("%s: grabbing actor %s"), *GetName(), *InteractableActor->GetName());

	InteractingActor = InteractableActor;
	InteractingActor->BeginGrab(this);

	ReceiveOnGrab(InteractableActor);
}

void AHand::ReleaseActor()
{
	check(InteractingActor);

	UE_LOG(LogTemp, Warning, TEXT("%s: releasing held actor %s"), *GetName(), *InteractingActor->GetName());

	InteractingActor->EndGrab(this);

	// Finished interacting, hand not attached
	if (InteractingActor->GetAttachedHand() != this)
	{
		InteractingActor = nullptr;
	}

	ReceiveOnReleaseHeldActor(InteractingActor);
}

