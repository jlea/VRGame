// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "Firearm.generated.h"

class USkeletalMeshComponent;
class USoundCue;
class UParticleEmitter;
class AProjectile;
class AMagazine;
class AExtendedCharacter;
class UAnimMontage;
class ACartridge;
class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFirearmMagazineEvent, AFirearm*, Firearm, AMagazine*, Magazine);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFirearmEvent, AFirearm*, Firearm);

UENUM(BlueprintType)
enum class EChamberedRoundStatus : uint8
{
	NoRound,
	Fresh,
	Spent
}; 

UENUM(BlueprintType)
enum class EFirearmAmmoLoadType : uint8
{
	SemiAutomatic,
	Automatic,
	PumpAction
};

UENUM(BlueprintType)
enum class EAmmoPreviewStatus : uint8
{
	None,
	OutOfRange,
	WithinRange
};

UCLASS()
class VRTEST_API AFirearm : public AInteractableActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFirearm();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//////////////////////////////////////////////////////////////////////////
	//	VR

	virtual bool CanGrab(const AHand* Hand) override;

	virtual void OnBeginInteraction(AHand* Hand) override;
	virtual void OnEndInteraction(AHand* Hand) override;
	virtual void OnBeginPickup(AHand* Hand) override;
	virtual void OnDrop(AHand* Hand) override;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	AMagazine*	GetLoadedMagazine() { return LoadedMagazine; }
	
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsCompatibleMagazine(AMagazine* Magazine);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsReadyToLoadMagazine(AMagazine* Magazine);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void LoadMagazine(AMagazine* NewMagazine);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool EjectLoadedMagazine();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Animations)")
	FName HandAttachSocket;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	TArray<FName> PickupBones;

	//////////////////////////////////////////////////////////////////////////

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool AttachToCharacter(AExtendedCharacter* NewCharacter);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void DetachFromCharacter();

	UFUNCTION(BlueprintPure)
	virtual bool CanFire();

	virtual void Fire();

	/* Loads a free round from our magazine into the chamber */
	void LoadRoundFromMagazine();

	/* Removes the chambered round and spawns the cartridge */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EjectRound();

	UFUNCTION(BlueprintImplementableEvent)
	void OnFire();

	UFUNCTION(BlueprintImplementableEvent)
	void OnDryFire();

	// Find the closest bone
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Mesh")
	USkeletalMeshComponent* FirearmMesh;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Mesh")
	UStaticMeshComponent* MagazinePreviewMesh;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UBoxComponent*	MagazineCollisionBox;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Animations)")
	FName MagazineAttachSocket;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Animations)")
	FName CharacterAttachSocket;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Animations)")
	FName ShellAttachSocket;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	FName HandleBone;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	FName StockBone;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	FName MagazineBone;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	FName BoltBone;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	FName MuzzleBone;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	FName TriggerBone;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Animation")
	UAnimMontage* FireAnimation;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon FX")
	USoundCue* DryFireSound;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon FX")
	USoundCue* FireSound;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon FX")
	UParticleSystem* FireParticle;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Weapon FX")
	UHapticFeedbackEffect_Base* RecoilEffect;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon FX")
	TSubclassOf<ACartridge> CartridgeClass;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon FX")
	float CartridgeEjectVelocity;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Gameplay")
	EFirearmAmmoLoadType AmmoLoadType;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Gameplay")
	float FireRate;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Gameplay")
	float Spread;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Gameplay")
	int32 BulletsPerShot;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Ammo")
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapon Ammo")
	bool bStartWithMagazine;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Ammo")
	TSubclassOf<AMagazine> DefaultMagazineClass;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Ammo")
	bool bHasInternalMagazine;

	EChamberedRoundStatus ChamberedRoundStatus;

	//////////////////////////////////////////////////////////////////////////
	//	 UI

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	EAmmoPreviewStatus AmmoPreviewStatus;

	void SetAmmoPreviewStatus(EAmmoPreviewStatus NewPreviewStatus);

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void OnNearbyHeldAmmoChanged(EAmmoPreviewStatus NewPreviewStatus);

	//////////////////////////////////////////////////////////////////////////
	//	Delegates

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FFirearmMagazineEvent OnMagazineLoaded;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FFirearmMagazineEvent OnMagazineEjected;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FFirearmEvent OnFirearmPickedUp;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FFirearmEvent OnFirearmDropped;

protected:
	UPROPERTY()
	AExtendedCharacter*	AttachedCharacter;

	UPROPERTY()
	AMagazine*	LoadedMagazine;

	bool bHasFired;
	bool bTriggerDown;
	float LastFireTime;

	bool bEjectRoundOnFire;
};
