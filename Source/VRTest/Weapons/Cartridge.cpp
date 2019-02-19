// Fill out your copyright notice in the Description page of Project Settings.

#include "Cartridge.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ACartridge::ACartridge()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>("CasingMesh");
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetCollisionProfileName(TEXT("Debris"));
	CasingMesh->SetNotifyRigidBodyCollision(true);
	CasingMesh->OnComponentHit.AddDynamic(this, &ACartridge::OnHit);

	RootComponent = CasingMesh;
}

// Called when the game starts or when spawned
void ACartridge::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(15.0f);

	if (bEmpty)
	{

	}
}

void ACartridge::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
}

