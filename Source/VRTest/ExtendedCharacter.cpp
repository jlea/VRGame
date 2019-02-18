// Fill out your copyright notice in the Description page of Project Settings.

#include "ExtendedCharacter.h"
#include "Sound/SoundCue.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AExtendedCharacter::AExtendedCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MaxHealth = 100;
	bDead = false;
}

// Called when the game starts or when spawned
void AExtendedCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
}

// Called every frame
void AExtendedCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AExtendedCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

bool AExtendedCharacter::ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	if (bDead)
	{
		return false;
	}

	return Super::ShouldTakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

float AExtendedCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float BaseDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (!bDead)
	{
		CurrentHealth -= BaseDamage;
		if (CurrentHealth <= 0.0f)
		{
			Kill();
		}
	}

	return BaseDamage;
}

void AExtendedCharacter::Kill()
{
	check(!bDead);

	GetController()->UnPossess();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->Deactivate();

	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);

	UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation(), GetActorRotation());

	OnKilled();
	OnKilledDelegate.Broadcast(this);

	bDead = true;
}

