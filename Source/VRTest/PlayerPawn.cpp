// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerPawn.h"
#include "Firearm.h"
#include "Classes/Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Hand.h"

// Sets default values
APlayerPawn::APlayerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>("SceneRoot");
	RootComponent = SceneRoot;

	VROrigin = CreateDefaultSubobject<USceneComponent>("VROrigin");
	VROrigin->SetupAttachment(SceneRoot);

	ScopeCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>("ScopeRender");

	ScopeInterpSpeed = 5.0f;
	CameraHeightOffset = 0.0f;
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
}

// Called every frame
void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ScopeFirearm)
	{
		FVector MuzzleLocation = ScopeFirearm->FirearmMesh->GetBoneLocation(ScopeFirearm->MuzzleBone, EBoneSpaces::ComponentSpace);
		FRotator MuzzleRotation = ScopeFirearm->FirearmMesh->GetBoneQuaternion(ScopeFirearm->MuzzleBone, EBoneSpaces::ComponentSpace).Rotator();

		LastScopePosition = FMath::VInterpTo(LastScopePosition, MuzzleLocation, DeltaTime, ScopeInterpSpeed);
		LastScopeRotation = FMath::RInterpTo(LastScopeRotation, MuzzleRotation, DeltaTime, ScopeInterpSpeed);

		ScopeCaptureComponent->SetRelativeRotation(MuzzleRotation);
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

void APlayerPawn::SetScopeFirearm(AFirearm* Firearm)
{

}

