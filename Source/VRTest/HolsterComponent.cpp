// Fill out your copyright notice in the Description page of Project Settings.

#include "HolsterComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "InteractableActor.h"

UHolsterComponent::UHolsterComponent()
{
	OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnEndOverlap);

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

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

		FBox Box = InteractableActor->GetComponentsBoundingBox(false);

		if (MyBounds.IsInside(Box))
		{
			ValidHoveredActors.Add(InteractableActor);
			
			DrawDebugBox(GetWorld(), Box.GetCenter(), Box.GetExtent(), FColor::Green, false, 0.1f, SDPG_World, 2.0f);
		}
		else
		{
			DrawDebugBox(GetWorld(), Box.GetCenter(), Box.GetExtent(), FColor::Red, false, 0.1f, SDPG_World, 2.0f);
		}
	}
}

void UHolsterComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
}

void UHolsterComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
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
