// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "ExtendedCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AProjectile::AProjectile()
{
	CollisionSphere = CreateDefaultSubobject<USphereComponent>("CollisionSphere");
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionProfileName(TEXT("BlockAll"));
	RootComponent = CollisionSphere;

	MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	MovementComponent->SetUpdatedComponent(CollisionSphere);
	MovementComponent->InitialSpeed = 6000.0f;
	MovementComponent->MaxSpeed = 6000.0f;

	ImpactForceScale = 5000.0f;
	Damage = 20.0f;

	CollisionSphere->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	CollisionSphere->IgnoreActorWhenMoving(GetOwner(), true);
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, GetActorLocation());

	UParticleSystem* FinalImpactParticle = ImpactParticle;

	auto CharacterImpact = Cast<AExtendedCharacter>(OtherActor);
	if (CharacterImpact)
	{
		FinalImpactParticle = FleshImpactParticle;
	}

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	UGameplayStatics::ApplyPointDamage(OtherActor, Damage, NormalImpulse, Hit, GetInstigatorController(), this, DamageType);

	if (OtherComp)
	{
		OtherComp->AddImpulse(-Hit.ImpactNormal * ImpactForceScale);
	}

	Destroy();
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
