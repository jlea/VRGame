// Fill out your copyright notice in the Description page of Project Settings.

#include "Hand.h"
#include "MotionControllerComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Character/PlayerPawn.h"
#include "DrawDebugHelpers.h"
#include "Gamemode/VRGameState.h"
#include "Engine.h"
#include "World/TeleportDestination.h"
#include "WidgetInteractionComponent.h"
#include "Interactable/InteractionHelper.h"
#include "Interactable/InteractableActor.h"

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

	WeaponMountOrigin = CreateDefaultSubobject<USceneComponent>("WeaponMount");
	WeaponMountOrigin->SetupAttachment(SphereCollision);

	bUseProjectileTeleport = false;

	bWantsGrab = false;
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

	CachedMeshTransform = HandMesh->GetRelativeTransform();

	// Create some interaction helpers
	const int NumHelpers = 5;
	for (int i = 0; i < NumHelpers; i++)
	{
		AInteractionHelper* InteractionHelper = GetWorld()->SpawnActor<AInteractionHelper>(InteractionHelperClass, GetTransform());
		InteractionHelper->AttachToComponent(HandOrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		
		InteractionHelpers.Add(InteractionHelper);
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

	if (bWantsTeleport)
	{
		UpdateTeleport();
	}
	
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

	UpdateHelpers();
}


void AHand::UpdateNearbyActors()
{
	auto VRGameState = Cast<AVRGameState>(GetWorld()->GetGameState());
	if (!VRGameState)
	{
		return;
	}

	AInteractableActor* OldNearbyActor = ClosestNearbyActor;

	NearbyActors.Empty();
	NearbyInteractableActors.Empty();

	const float SearchDistance = SphereCollision->GetScaledSphereRadius();
	const FVector HandLocation = GetHandSelectionOrigin();

	EInteractPriority BestPriority = EInteractPriority::Low;
	float ClosestActorValue = FLT_MAX;
	AInteractableActor* ClosestActor = nullptr;

	for (auto Actor : VRGameState->GetInteractableActors())
	{
		if (!Actor)
		{
			continue;
		}

		if (!SphereCollision->IsOverlappingActor(Actor))
		{
			continue;
		}

		NearbyActors.Add(Actor);

		TArray<FInteractionHelperReturnParams> Params;
		Actor->GetInteractionConditions(this, Params);

		if (Params.Num() == 0)
		{
			continue;
		}

		NearbyInteractableActors.Add(Actor);

		if (Actor->InteractPriority < BestPriority)
		{
			continue;
		}

		const FVector DirectionToActor = (Actor->GetActorLocation() - HandLocation).GetSafeNormal();
		float DistanceToActor = (Actor->GetActorLocation() - HandLocation).Size();

		const float DotProduct = FVector::DotProduct(GetActorForwardVector(), DirectionToActor);

		// Higher priority, use this
		if (Actor->InteractPriority > BestPriority)
		{
			BestPriority = Actor->InteractPriority;

			ClosestActor = Actor;
			ClosestActorValue = DistanceToActor;
		}
		else
		{
			if (DistanceToActor < ClosestActorValue)
			{
				ClosestActor = Actor;
				ClosestActorValue = DistanceToActor;
			}
		}
	}

	// If we are holding something already, just use that 
	if (InteractingActor)
	{
		ClosestNearbyActor = InteractingActor;
		return;
	}

	ClosestNearbyActor = ClosestActor;

	if (ClosestNearbyActor != OldNearbyActor)
	{
		// Reset our interaction helpers
		for (auto InteractionHelper : InteractionHelpers)
		{
			if (!InteractionHelper)
			{
				continue;
			}

			InteractionHelper->SetHidden();
		}

		OnHoverActorChanged(ClosestNearbyActor);
	}
}

void AHand::UpdateHelpers()
{
	AInteractableActor* HelperActor = ClosestNearbyActor;
	if (!HelperActor)
	{
		AHand* TestHand = nullptr;

		// No nearby actor.. see what's in our other hand
		if (HandType == EControllerHand::Left)
		{
			TestHand = GetPlayerPawn()->RightHand;
		}
		else
		{
			TestHand = GetPlayerPawn()->LeftHand;
		}

		if (TestHand)
		{
			if (TestHand->GetInteractingActor())
			{
				HelperActor = TestHand->GetInteractingActor();
			}
		}
	}

	float BestDistance = FLT_MAX;
	AInteractionHelper* BestHelper = nullptr;
	TArray<FInteractionHelperReturnParams> HelperParams;

	if (HelperActor)
	{
		HelperActor->GetInteractionConditions(this, HelperParams);

		// Sort our helpers to find the closest
		for (int i = 0; i < InteractionHelpers.Num(); i++)
		{
			if (!HelperParams.IsValidIndex(i))
			{
				continue;
			}

			FInteractionHelperReturnParams& Param = HelperParams[i];
			AInteractionHelper* InteractionHelper = InteractionHelpers[i];

			const bool bValidHelper = Param.HelperState == EInteractionHelperState::Valid || Param.HelperState == EInteractionHelperState::Active;
			if (bValidHelper)
			{
				const float DistanceToHelper = FVector::Dist(Param.WorldLocation, GetHandSelectionOrigin());
				if (DistanceToHelper < BestDistance)
				{
					BestDistance = DistanceToHelper;
					BestHelper = InteractionHelper;
				}
			}
		}
	}

	// Don't update the helper if we are already interacting
	const bool bUpdateBestHelper = InteractingActor == nullptr;
	if (bUpdateBestHelper)
	{
		ActiveInteractionHelper = BestHelper;
	}

	// Update the state of all helpers once we have the interaction helper
	for (int i = 0; i < InteractionHelpers.Num(); i++)
	{
		AInteractionHelper* InteractionHelper = InteractionHelpers[i];
		if (!InteractionHelper)
		{
			continue;
		}

		if (!HelperParams.IsValidIndex(i))
		{
			InteractionHelper->SetHidden();
			continue;
		}

		FInteractionHelperReturnParams& Param = HelperParams[i];

		if (ActiveInteractionHelper == InteractionHelper)
		{
			Param.HelperState = EInteractionHelperState::Active;
		}

		if (InteractingActor)
		{
			// Don't display helpers while we are interacting with something
			Param.bShouldRender = false;
		}

		InteractionHelper->SetHelperParams(Param);
	}
}

void AHand::OnDropPressed()
{
	if (InteractingActor)
	{
		InteractingActor->Drop(this);
		InteractingActor = nullptr;
	}
}

void AHand::OnGrabPressed()
{
	ReceiveOnGrabPressed();

	bWantsGrab = true;

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
	ReceiveOnGrabReleased();

	bWantsGrab = false;

	if (InteractingActor)
	{
		ReleaseActor();
	}
}
FVector AHand::GetHandSelectionOrigin() const
{
	return SphereCollision->GetComponentLocation();
}

void AHand::ResetMeshToOrigin()
{
	HandMesh->AttachToComponent(GetSphereComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	HandMesh->SetRelativeTransform(CachedMeshTransform);
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

void AHand::OnTeleportPressed()
{
	bWantsTeleport = true;
}

void AHand::OnTeleportReleased()
{
	TryTeleport();

	bWantsTeleport = false;
}

void AHand::OnDirectionalPadPressed(const EDirectionPadInput Direction)
{
	if (InteractingActor)
	{
		InteractingActor->DirectionalPad(this, Direction);
	}
}

void AHand::TryTeleport()
{
	if (bHasValidTeleportLocation)
	{
		if (bUseProjectileTeleport)
		{
			GetPlayerPawn()->SetActorLocation(ValidTeleportLocation);
			GetPlayerPawn()->OnTeleported(ValidTeleportLocation);
		}
		else
		{
			auto TeleportDestination = Cast<ATeleportDestination>(TeleportLineTrace.GetActor());
			if (TeleportDestination)
			{
				TeleportDestination->TeleportToDestination(GetPlayerPawn(), this);
			}
		}
	}
}

void AHand::UpdateTeleport()
{
	bHasValidTeleportLocation = false;

	FHitResult TeleporTrace;
	
	TArray<AActor*>	ActorsToIgnore;
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(GetPlayerPawn());
	ActorsToIgnore.Add(InteractingActor);

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActors(ActorsToIgnore);

	if(bUseProjectileTeleport)
	{
		FPredictProjectilePathParams TeleportParams;
		TeleportParams.ProjectileRadius = 2.0f;
		TeleportParams.StartLocation = SphereCollision->GetComponentLocation();
		TeleportParams.bTraceWithChannel = true;
		TeleportParams.bTraceWithCollision = true;
		TeleportParams.TraceChannel = ECC_Visibility;
		TeleportParams.DrawDebugType = EDrawDebugTrace::ForOneFrame;
		TeleportParams.LaunchVelocity = SphereCollision->GetForwardVector() * TeleportProjectileVelocity;
		TeleportParams.ActorsToIgnore = ActorsToIgnore;

		UGameplayStatics::PredictProjectilePath(this, TeleportParams, TeleportResult);

		// Now trace downwards
		FHitResult PostTraceHit;
		GetWorld()->LineTraceSingleByChannel(PostTraceHit, TeleportResult.LastTraceDestination.Location, (TeleportResult.LastTraceDestination.Location + FVector::UpVector * -1000.0f), ECC_Visibility);
		if (PostTraceHit.IsValidBlockingHit())
		{
			bHasValidTeleportLocation = true;
			ValidTeleportLocation = PostTraceHit.ImpactPoint;
		}
	}
	else
	{
		// Do a line trace, see if in interacts with our teleport destination
		const FVector TraceStart = SphereCollision->GetComponentLocation();
		const FVector TraceEnd = TraceStart + SphereCollision->GetForwardVector() * 100000.0f;

		auto LastTeleportDestination = Cast<ATeleportDestination>(TeleportLineTrace.GetActor());

		GetWorld()->LineTraceSingleByChannel(TeleportLineTrace, TraceStart, TraceEnd, ECC_GameTraceChannel4, CollisionParams);

		if (TeleportLineTrace.IsValidBlockingHit())
		{
			auto TeleportDestination = Cast<ATeleportDestination>(TeleportLineTrace.GetActor());
			if (TeleportDestination)
			{
				if (!TeleportDestination->bHovered)
				{
					TeleportDestination->bHovered = true;
					TeleportDestination->OnTeleportHovered(GetPlayerPawn(), this);
				}

				bHasValidTeleportLocation = true;
				return;
			}
			else
			{
				bHasValidTeleportLocation = false;
			}
		}
		else
		{
			bHasValidTeleportLocation = false;
		}

		if (LastTeleportDestination)
		{
			if (LastTeleportDestination->bHovered)
			{
				LastTeleportDestination->OnTeleportUnhovered(GetPlayerPawn(), this);
			}

			LastTeleportDestination->bHovered = false;
		}
	}
}
