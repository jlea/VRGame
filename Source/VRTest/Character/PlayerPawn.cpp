// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerPawn.h"
#include "Weapons/Firearm.h"
#include "Kismet/GameplayStatics.h"
#include "Classes/Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "UI/VRGameViewportClient.h"
#include "Interactable/Hand.h"

// Sets default values
APlayerPawn::APlayerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>("SceneRoot");
	RootComponent = SceneRoot;

	VROrigin = CreateDefaultSubobject<USceneComponent>("VROrigin");
	VROrigin->SetupAttachment(SceneRoot);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(VROrigin);

	ScopeCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>("ScopeRender");

	MaxHealth = 500;

	ScopeInterpSpeed = 5.0f;
	CameraHeightOffset = 0.0f;

	TeamId = FGenericTeamId(0);
}

// Called when the game starts or when spawned
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
	// Spawn our hands
	LeftHand = GetWorld()->SpawnActorDeferred<AHand>(HandClass, FTransform::Identity, this);
	LeftHand->HandType = EControllerHand::Left;
	LeftHand->FinishSpawning(FTransform::Identity);
	LeftHand->AttachToComponent(VROrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	RightHand = GetWorld()->SpawnActorDeferred<AHand>(HandClass, FTransform::Identity, this);
	RightHand->HandType = EControllerHand::Right;
	RightHand->FinishSpawning(FTransform::Identity);
	RightHand->AttachToComponent(VROrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	VROrigin->AddLocalOffset(FVector(0, 0, CameraHeightOffset));

	ScopeCaptureComponent->Deactivate();

	LastDamageTimestamp = DeathTimestamp = -FLT_MAX;
	CurrentHealth = MaxHealth;
	bDead = false;
}

void APlayerPawn::Reset()
{
	Super::Reset();

	LastDamageTimestamp = DeathTimestamp = -FLT_MAX;
	CurrentHealth = MaxHealth;
	bDead = false;
}

// Called every frame
void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ScopeFirearm)
	{
		FVector MuzzleLocation = ScopeFirearm->FirearmMesh->GetSocketLocation(ScopeFirearm->MuzzleBone);
		FRotator MuzzleRotation = ScopeFirearm->FirearmMesh->GetSocketRotation(ScopeFirearm->MuzzleBone);

		if (ScopeInterpSpeed > 0.0f)
		{
			LastScopePosition = FMath::VInterpTo(LastScopePosition, MuzzleLocation, DeltaTime, ScopeInterpSpeed);
			LastScopeRotation = FMath::RInterpTo(LastScopeRotation, MuzzleRotation, DeltaTime, ScopeInterpSpeed);
		}
		else
		{
			LastScopePosition = MuzzleLocation;
			LastScopeRotation = MuzzleRotation;
		}

		ScopeCaptureComponent->SetWorldLocation(LastScopePosition);
		ScopeCaptureComponent->SetWorldRotation(LastScopeRotation);
	}
}

// Called to bind functionality to input
void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent->BindAction("DropLeft", IE_Pressed, this, &APlayerPawn::DropLeftPressed);
	InputComponent->BindAction("DropRight", IE_Pressed, this, &APlayerPawn::DropRightPressed);

	InputComponent->BindAction("GrabLeft", IE_Pressed, this, &APlayerPawn::GrabLeftPressed);
	InputComponent->BindAction("GrabLeft", IE_Released, this, &APlayerPawn::GrabLeftReleased);

	InputComponent->BindAction("GrabRight", IE_Pressed, this, &APlayerPawn::GrabRightPressed);
	InputComponent->BindAction("GrabRight", IE_Released, this, &APlayerPawn::GrabRightReleased);

	InputComponent->BindAction("Teleport", IE_Pressed, this, &APlayerPawn::TeleportPressed);
	InputComponent->BindAction("Teleport", IE_Released, this, &APlayerPawn::TeleportReleased);
}

FVector APlayerPawn::GetTargetLocation(AActor* RequestedBy /* = nullptr */) const
{
	return CameraComponent->GetComponentLocation();
}

void APlayerPawn::DropLeftPressed()
{
	LeftHand->OnDropPressed();
}

void APlayerPawn::DropRightPressed()
{
	RightHand->OnDropPressed();
}

void APlayerPawn::GrabLeftPressed()
{
	LeftHand->OnGrabPressed();
}

void APlayerPawn::GrabLeftReleased()
{
	LeftHand->OnGrabReleased();
}

void APlayerPawn::GrabRightPressed()
{
	RightHand->OnGrabPressed();
}

void APlayerPawn::GrabRightReleased()
{
	RightHand->OnGrabReleased();
}

void APlayerPawn::TeleportPressed()
{
	RightHand->OnTeleportPressed();
}

void APlayerPawn::TeleportReleased()
{
	RightHand->OnTeleportReleased();
}

void APlayerPawn::SetScopeFirearm(AFirearm* Firearm)
{
	ScopeFirearm = Firearm;
	
	if (ScopeFirearm)
	{
		ScopeCaptureComponent->Activate();
	}
	else
	{
		ScopeCaptureComponent->Deactivate();
	}
}

float APlayerPawn::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float BaseDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	float FinalDamage = BaseDamage;

	if (!bDead)
	{
		CurrentHealth -= FinalDamage;
		LastDamageTimestamp = GetWorld()->GetTimeSeconds();

		if (CurrentHealth <= 0.0f)
		{
			Kill(EventInstigator, DamageCauser, DamageEvent);
		}
		else
		{
			OnHealthChanged(-FinalDamage);
		}
	}
	
	return FinalDamage;
}

void APlayerPawn::Kill(AController* Killer, AActor *DamageCauser, struct FDamageEvent const& DamageEvent)
{
	bDead = true;
	DeathTimestamp = GetWorld()->GetTimeSeconds();

	FHitResult HitResult;
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageEvent = (const FPointDamageEvent*)&DamageEvent;
		if (PointDamageEvent)
		{
			HitResult = PointDamageEvent->HitInfo;
		}
	}

	OnKilledDelegate.Broadcast(this, Killer, HitResult);

	LeftHand->OnDropPressed();
	RightHand->OnDropPressed();

	OnKilled(Killer, DamageCauser, DamageEvent);
}

