// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ExtendedCharacter.generated.h"

class USoundCue;
class AWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCharacterKilledDelegate, AExtendedCharacter*, Character, AController*, Killer);

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
	//	Damage (Gameplay)

	virtual bool ShouldTakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void PlayDeathAnimation();
	virtual bool ShouldRagdollOnDeath(FHitResult Hit);

	UFUNCTION(BlueprintCallable, Category = "Character")
	void Kill(AController* Killer, AActor *DamageCauser, struct FDamageEvent const& DamageEvent);

	UFUNCTION(BlueprintImplementableEvent, Category = "Character")
	void OnKilled(AController* Killer, AActor *DamageCauser, struct FDamageEvent const& DamageEvent);

	UFUNCTION()
	void Ragdoll();

	UPROPERTY()
	FTimerHandle TimerHandle_Ragdoll;

	//////////////////////////////////////////////////////////////////////////
	//	Damage (FX)


	/* Sounds played when we die */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Sounds)
	USoundCue* DeathSound;

	/* Sounds played when we die */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Sounds)
	USoundCue* PainSound;

	UPROPERTY()
	UAudioComponent* CurrentDialogue;

	UFUNCTION(BlueprintCallable, Category = Sounds)
	virtual void PlayDialogueSound(USoundCue* Sound);

	UFUNCTION(BlueprintCallable, Category = Damage)
	virtual void Bleed(FName Bone);

	UFUNCTION(BlueprintCallable, Category = Damage)
	virtual void SeverLimb(FHitResult Hit);

	UFUNCTION(BlueprintImplementableEvent, Category = Damage)
	void OnSeverLimb(FHitResult Hit);

	/** Effect to spawn when we kill something */
	UPROPERTY(EditAnywhere, Category = Damage)
	UParticleSystem* BleedEffect;

	UPROPERTY(BlueprintReadOnly, Category = Damage)
	TArray<FName> SeveredLimbs;


	//////////////////////////////////////////////////////////////////////////
	//	Weapons

	//UPROPERTY()
	//AWeapon*	EquippedWeapon;
	
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

	UPROPERTY(EditAnywhere, Category = "Animation")
	TArray<UAnimMontage*>	DeathAnimations;

	//////////////////////////////////////////////////////////////////////////
	//	Variables

	UPROPERTY(EditAnywhere, Category = "Character")
	int32 MaxHealth;

	UPROPERTY()
	int32 CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	bool bDead;

	UPROPERTY(BlueprintAssignable)
	FCharacterKilledDelegate OnKilledDelegate;
};
