// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ExtendedCharacter.generated.h"

class USoundCue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharacterKilledDelegate, AExtendedCharacter*, Character);

UCLASS()
class VRTEST_API AExtendedCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AExtendedCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//////////////////////////////////////////////////////////////////////////
	//	Functions

	virtual bool ShouldTakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Character")
	void Kill();

	UFUNCTION(BlueprintImplementableEvent, Category = "Character")
	void OnKilled();
	
	//////////////////////////////////////////////////////////////////////////
	//	Animation

	bool IsLockedToAnimation() { return false; }

	virtual void UpdateCharacterBodyTwist(float DeltaSeconds);

	/* Our stored actor rotation (its not modified until we reach a twist threshold)*/
	UPROPERTY()
	FRotator StoredMeshRotation;

	/** Difference between our actor rotation and our aiming rotation */
	UPROPERTY(BlueprintReadOnly, Category = Animation)
	float AimYawDelta;

	/** Difference between our actor rotation and our aiming rotation */
	UPROPERTY(BlueprintReadOnly, Category = Animation)
	float AimPitchDelta;

	/* )*/
	float CurrentControllerYaw;

	/* */
	float CurrentControllerPitch;

	//////////////////////////////////////////////////////////////////////////
	//	Variables

	UPROPERTY(EditAnywhere, Category = "Character")
	USoundCue*	PainSound;

	UPROPERTY(EditAnywhere, Category = "Character")
	USoundCue*	DeathSound;

	UPROPERTY(EditAnywhere, Category = "Character")
	int32 MaxHealth;

	UPROPERTY()
	int32 CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	bool bDead;

	UPROPERTY(BlueprintAssignable)
	FCharacterKilledDelegate OnKilledDelegate;
};
