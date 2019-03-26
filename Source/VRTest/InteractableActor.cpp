// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractableActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "VRGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Hand.h"

// Sets default values
AInteractableActor::AInteractableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AttachedHand = nullptr;

	InteractPriority = EInteractPriority::Low;

	bDropOnRelease = true;
	bAttachToSocket = false;
	bXAxisOriented = false;
}

// Called when the game starts or when spawned
void AInteractableActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Cache it
	auto VRGameState = Cast<AVRGameState>(GetWorld()->GetGameState());
	if (VRGameState)
	{
		VRGameState->AddInteractableActor(this);
	}
}

// Called every frame
void AInteractableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AInteractableActor::CanHolster() const
{
	return true;
}

void AInteractableActor::DrawHUD(AHUD* HUD, AHand* InteractingHand)
{
	// Do anything 
}

void AInteractableActor::BeginGrab(AHand* Hand)
{
	if (AttachedHand)
	{
		OnBeginInteraction(Hand);
	}
	else
	{
		OnBeginPickup(Hand);
	}
}

void AInteractableActor::EndGrab(AHand* Hand)
{
	if (Hand == AttachedHand)
	{
		if (bDropOnRelease)
		{
			OnDrop(Hand);
		}
	}

	if (InteractingHands.Contains(Hand))
	{
		OnEndInteraction(Hand);
	}
}

void AInteractableActor::Drop(AHand* Hand)
{
	if (Hand == AttachedHand)
	{
		OnDrop(Hand);
	}

	if (InteractingHands.Contains(Hand))
	{
		OnEndInteraction(Hand);
	}
}

AHand* AInteractableActor::GetBestInteractingHand()
{
	for (auto PossibleInteractingHand : InteractingHands)
	{
		if (AttachedHand)
		{
			// Skip the attached hand 
			if (PossibleInteractingHand == AttachedHand)
			{
				continue;
			}
		}

		return PossibleInteractingHand;
	}

	return nullptr;
}

bool AInteractableActor::CanInteract(const AHand* InteractingHand, FInteractionHelperReturnParams& ReturnParams) const
{
	// Can't grab.. already holding
	if (AttachedHand && AttachedHand == InteractingHand)
	{
		return false;
	}

	ReturnParams.Location = GetActorLocation();
	return true;
}

void AInteractableActor::OnBeginPickup(AHand* Hand)
{
	check(!AttachedHand);

	AttachedHand = Hand;

	auto RootPrimitive = Cast<UPrimitiveComponent>(RootComponent);
	if (RootPrimitive)
	{
		RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		RootPrimitive->SetSimulatePhysics(false);
	}

	if (bAttachToSocket)
	{
		AttachToComponent(Hand->GetHandMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandAttachSocket);
		
		if (Hand->HandType == EControllerHand::Left)
		{
			if (bXAxisOriented)
			{
				FRotator NewRelativeRotation = RootComponent->RelativeRotation;
				NewRelativeRotation.Roll += 180.0f;
				RelativeOffset = NewRelativeRotation;

				SetActorRelativeRotation(NewRelativeRotation);
			}
			else
			{
				FRotator NewRelativeRotation = RootComponent->RelativeRotation;
				NewRelativeRotation.Pitch += 180.0f;
				RelativeOffset = NewRelativeRotation;

				SetActorRelativeRotation(NewRelativeRotation);
			}
		}

		// Once in place, attach to the mount
		AttachToComponent(Hand->GetWeaponMountOrigin(), FAttachmentTransformRules::KeepWorldTransform);
	}
	else
	{
		AttachToComponent(Hand->GetHandMesh(), FAttachmentTransformRules::KeepWorldTransform);
	}

	UGameplayStatics::SpawnSoundAttached(PickupSound, GetRootComponent());
	
	ReceiveOnBeginPickup(Hand);
	OnPickedUpDelegate.Broadcast(this);
}

void AInteractableActor::OnBeginInteraction(AHand* Hand)
{
	check(!InteractingHands.Contains(Hand));

	//GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Green, FString::Printf(TEXT("%s: Interacting"), *GetName()), true, FVector2D(3.0f, 3.0f));

	// Already attached, so assume this is an interaction
	InteractingHands.Add(Hand);

	ReceiveOnBeginInteraction(Hand);
	OnInteractionStart.Broadcast(this);
}

void AInteractableActor::OnDrop(AHand* Hand)
{
	check(AttachedHand);

	AttachedHand = nullptr;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	auto RootPrimitive = Cast<UPrimitiveComponent>(RootComponent);
	if (RootPrimitive)
	{
		RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		RootPrimitive->SetSimulatePhysics(true);
	}

	// BP event
	ReceiveOnDrop(Hand);
	OnDropDelegate.Broadcast(this);
}

void AInteractableActor::OnEndInteraction(AHand* Hand)
{
	check(InteractingHands.Contains(Hand));

	InteractingHands.Remove(Hand);

	// BP event
	ReceiveOnEndInteraction(Hand);
	OnInteractionEnd.Broadcast(this);
}
