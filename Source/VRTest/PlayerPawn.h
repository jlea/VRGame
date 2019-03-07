// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawn.generated.h"

class AHand;
class AFirearm;
class UCameraComponent;
class UTextureRenderTarget2D;

UCLASS()
class VRTEST_API APlayerPawn : public APawn, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FGenericTeamId TeamId;

	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetTargetLocation(AActor* RequestedBy /* = nullptr */) const override;

	//////////////////////////////////////////////////////////////////////////
	//	Input

	void DropLeftPressed();
	void DropRightPressed();

	void GrabLeftPressed();
	void GrabLeftReleased();

	void GrabRightPressed();
	void GrabRightReleased();

	void TeleportPressed();
	void TeleportReleased();

	void BulletTimePressed();
	void BulletTimeReleased();

	//////////////////////////////////////////////////////////////////////////
	//	Misc

	UFUNCTION(BlueprintImplementableEvent, Category = "Teleport")
	void OnTeleported(const FVector Destination);

	UFUNCTION(BlueprintCallable, Category = "VR")
	void SetScopeFirearm(AFirearm* Firearm);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	USceneCaptureComponent2D* ScopeCaptureComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	USceneComponent*	SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	USceneComponent*	VROrigin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR")
	float CameraHeightOffset;

	UPROPERTY(EditAnywhere, Category = "Hand")
	TSubclassOf<AHand>	HandClass;

	UPROPERTY()
	AHand* LeftHand;

	UPROPERTY()
	AHand* RightHand;

	UPROPERTY()
	AFirearm* ScopeFirearm;

	UPROPERTY(EditAnywhere, Category = "Hand (Weapons)")
	float ScopeInterpSpeed;

	FVector LastScopePosition;
	FRotator LastScopeRotation;

	//////////////////////////////////////////////////////////////////////////
	//	Bullet time

	UFUNCTION(BlueprintImplementableEvent, Category = "Bullet Time")
	void OnBulletTimeBegin();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bullet Time")
	void OnBulletTimeFinish();

	UPROPERTY(BlueprintReadOnly, Category = "Bullet Time")
	bool bBulletTime;

	UPROPERTY(BlueprintReadOnly, Category = "Bullet Time")
	float CurrentTimeDilation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Time")
	float BulletTimeInterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Time")
	float BulletTimeModifier;

	//////////////////////////////////////////////////////////////////////////
	//	Damage
	
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Damage")
	void OnHealthChanged(float Amount);

	void Kill(AController* Killer, AActor *DamageCauser, struct FDamageEvent const& DamageEvent);

	UFUNCTION(BlueprintImplementableEvent, Category = "Damage")
	void OnKilled(AController* Killer, AActor *DamageCauser, struct FDamageEvent const& DamageEvent);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	int32 MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	int32 CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	bool bDead;

	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float LastDamageTimestamp;

	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float DeathTimestamp;
};
