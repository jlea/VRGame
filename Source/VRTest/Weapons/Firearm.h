// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/InteractableActor.h"
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
struct FDamageAnimation;

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

//TODO: Animation struct
UENUM(BlueprintType)
enum class EDamageDirection : uint8
{
	Left,
	Right,
	Front,
	Back
};

USTRUCT(BlueprintType)
struct FDamageAnimation
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	UAnimMontage* Animation;

	/* Direction this action is related to */
	UPROPERTY(EditAnywhere)
	EDamageDirection DamageDirection;

	FDamageAnimation()
	{
		DamageDirection = EDamageDirection::Front;
		Animation = nullptr;
	}
};

UCLASS()
class VRTEST_API AFirearm : public AInteractableActor
{
	friend class ACharacterAIController;

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

	virtual bool CanInteract(const AHand* InteractingHand, FInteractionHelperReturnParams& Params) const;

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

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetLoadedRounds();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void LoadMagazine(AMagazine* NewMagazine);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool EjectLoadedMagazine();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Grip)")
	bool bUseTwoHandedGrip;

	/* If specified, will allow two handed usage on the grip */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Grip)", meta = (EditCondition=bUseTwoHandedGrip))
	FName GripBone;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Grip)", meta = (EditCondition = bUseTwoHandedGrip))
	FName GripAttachSocket;

	//////////////////////////////////////////////////////////////////////////
	//	Firearm

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool AttachToCharacter(AExtendedCharacter* NewCharacter);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void DetachFromCharacter();

	UFUNCTION(BlueprintPure)
	virtual bool CanFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();

	/* Loads a free round from our magazine into the chamber */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void LoadRoundFromMagazine();

	/* Removes the chambered round and spawns the cartridge */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EjectRound();

	UFUNCTION(BlueprintImplementableEvent)
	void OnFire();

	UFUNCTION(BlueprintImplementableEvent)
	void OnDryFire();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Slide)")
	FName SlideStartSocket;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Slide)")
	FName SlideEndSocket;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Slide)")
	FName SlideAttachSocket;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Slide)")
	float SlideProgress;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Slide)")
	bool bOpenBolt;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Slide)")
	bool bSnapSlideForwardOnRelease;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(EditUsing=bSnapSlideForwardOnRelease), Category = "Weapon (Slide)")
	float SnapSlideSpeed;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Slide)")
	bool bEjectRoundOnSlide;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Mesh")
	USkeletalMeshComponent* FirearmMesh;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Mesh")
	UStaticMeshComponent* MagazinePreviewMesh;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UBoxComponent*	MagazineCollisionBox;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Interaction)")
	FName MagazineAttachSocket;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Interaction)")
	FName CharacterAttachSocket;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Interaction)")
	FName ShellAttachSocket;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon (Interaction)")
	FName MagazineBone;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon FX")
	FName MuzzleBone;

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

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Animation")
	TArray<UAnimMontage*> FireAnimation;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Animation")
	TArray<UAnimMontage*> FireAnimationCharacter;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Animation")
	TArray<FDamageAnimation> DamageAnimations;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Animation")
	TArray<UAnimMontage*> DeathAnimations;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Gameplay")
	EFirearmAmmoLoadType AmmoLoadType;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Gameplay")
	float FireRate;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Gameplay")
	float Spread;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon Gameplay")
	float SpreadAI;

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

	UFUNCTION(BlueprintImplementableEvent, Category = "Pump Action")
	void OnSlideBack();

	UFUNCTION(BlueprintImplementableEvent, Category = "Pump Action")
	void OnSlideForward();

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FFirearmEvent OnSlideForwardDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FFirearmEvent OnSlideBackDelegate;

protected:
	UPROPERTY()
	AExtendedCharacter*	AttachedCharacter;

	UPROPERTY()
	AMagazine*	LoadedMagazine;

	bool bHasFired;
	bool bTriggerDown;
	float LastFireTime;

	bool bHasSlidBack;

	bool bEjectRoundOnFire;

	bool bUsingSlide;
	bool bUsingGrip;
};
