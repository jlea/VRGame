// Fill out your copyright notice in the Description page of Project Settings.

#include "HolsterComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "InteractableActor.h"

UHolsterComponent::UHolsterComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	bHiddenInGame = false;
}

void UHolsterComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TSet<AActor*>	HoveredActors;
	GetOverlappingActors(HoveredActors, AInteractableActor::StaticClass());

	for (auto ValidHoveredActor : ValidHoveredActors)
	{
		auto RootPrimitive = Cast<UPrimitiveComponent>(ValidHoveredActor->GetRootComponent());
		if (RootPrimitive && RootPrimitive->IsSimulatingPhysics())
		{
			// Dropped item, scoop it up
			HolsterActor(ValidHoveredActor);
		}
	}

	ValidHoveredActors.Empty();

	auto MyBounds = Bounds.GetBox();

	// See which actors are hovered over us 
	for (auto HoveredActor : HoveredActors)
	{
		auto InteractableActor = Cast<AInteractableActor>(HoveredActor);
		if (!InteractableActor)
		{
			continue;
		}

		if (!InteractableActor->GetAttachedHand())
		{
			continue;
		}

		ValidHoveredActors.Add(InteractableActor);
	}

	if (LastValidActors != ValidHoveredActors.Num())
	{
		HolsterStateChanged.Broadcast();
	}

	LastValidActors = ValidHoveredActors.Num();
}

void UHolsterComponent::HolsterActor(AInteractableActor* ActorToHolster)
{
	auto RootPrimitive = Cast<UPrimitiveComponent>(ActorToHolster->GetRootComponent());
	if (!RootPrimitive)
	{
		return;
	}

	RootPrimitive->SetSimulatePhysics(false);
	
	ActorToHolster->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);

	UGameplayStatics::SpawnSoundAttached(ActorToHolster->HolsterSound, RootPrimitive);
}
