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

	InteractionHelper = CreateDefaultSubobject<UStaticMeshComponent>("InteractionHelper");
	InteractionHelper->SetupAttachment(HandOrigin);
	InteractionHelper->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

	FInteractionHelperReturnParams Params;

	const bool bShouldDrawHelper = ClosestNearbyActor && ClosestNearbyActor->CanInteract(this, Params) && Params.bRenderHelper;

	if(bShouldDrawHelper)
	{
		if (InteractionHelper->bHiddenInGame)
		{
			InteractionHelper->SetHiddenInGame(false, true);
		}

		InteractionHelper->SetWorldLocation(Params.Location);
	}
	else
	{
		if (!InteractionHelper->bHiddenInGame)
		{
			InteractionHelper->SetHiddenInGame(true, true);
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
	if (GetPlayerPawn()->bDead)
	{
		OnDeadGrabPressed();
		return;
	}

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
	if (GetPlayerPawn()->bDead)
	{
		OnDeadGrabReleased();
		return;
	}

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

	AInteractableActor*	OldNearbyActor = ClosestNearbyActor;

	NearbyActors.Empty();

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

		FInteractionHelperReturnParams Params;

		if (!Actor->CanInteract(this, Params))
		{
			continue;
		}

		NearbyActors.Add(Actor);

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
		OnHoverActorChanged(ClosestNearbyActor);
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
