// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class USoundCue;
class UParticleSystem;
class AImpactEffect;

UCLASS()
class VRTEST_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USphereComponent*	CollisionSphere;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UProjectileMovementComponent*	MovementComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projectile Gameplay")
	float						Damage;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projectile Gameplay")
	float						ImpactForceScale;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projectile Gameplay")
	TSubclassOf<UDamageType>	DamageType;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projectile Gameplay")
	TSubclassOf<AImpactEffect>	ImpactEffect;
};
