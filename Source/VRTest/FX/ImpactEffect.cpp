// Copyright 2010-2016 New World Interactive LLC and Epic Games, Inc. All Rights Reserved.

#include "ImpactEffect.h"
#include "Sound/SoundCue.h"
#include "Character/ExtendedCharacter.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"

AImpactEffect::AImpactEffect(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
}


void AImpactEffect::BeginPlay()
{
	Super::BeginPlay();

	UPhysicalMaterial* HitPhysMat = SurfaceHit.PhysMaterial.Get();
	EPhysicalSurface HitSurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitPhysMat);
	UParticleSystem* ImpactFX = GetImpactFX(HitSurfaceType);
	if (ImpactFX)
	{
		FRotator SpawnRotation = GetActorRotation();
		UParticleSystemComponent* Emitter = UGameplayStatics::SpawnEmitterAttached(ImpactFX, RootComponent, NAME_None,  FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget);
		if (Emitter)
		{
			if (SurfaceHit.GetActor())
			{
				ACharacter* HitCharacter = Cast<ACharacter>(SurfaceHit.GetActor());
				if (HitCharacter)
				{
					AttachToComponent(HitCharacter->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, NAME_None);
					//UE_LOG(LogTemp, Warning, TEXT("%s"), *SurfaceHit.GetActor()->GetName());
				}
			}
		}
	}

	//Play the sound
	USoundCue* ImpactSound = GetImpactSound(HitSurfaceType);
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	//Spawn the decal
	if (DefaultDecal.DecalMaterial)
	{
		FRotator RandomDecalRotation = SurfaceHit.ImpactNormal.Rotation().GetInverse();
		RandomDecalRotation.Roll = FMath::FRandRange(-180.0f, 180.0f);

		if (DefaultDecal.LifeSpan != 0.0f)
		{
			const FMatrix DecalInternalTransform = FRotationMatrix(FRotator(0.f, 90.0f, -90.0f));
			FVector AdjustedDecalSize = DecalInternalTransform.TransformVector(FVector(DefaultDecal.DecalSize, DefaultDecal.DecalSize, 1.0f));

			UGameplayStatics::SpawnDecalAttached(DefaultDecal.DecalMaterial, AdjustedDecalSize,
				SurfaceHit.Component.Get(), SurfaceHit.BoneName,
				SurfaceHit.ImpactPoint, RandomDecalRotation, EAttachLocation::KeepWorldPosition,
				DefaultDecal.LifeSpan);
		}

	}

	//Delete this actor after the decal wears off
	SetLifeSpan(DefaultDecal.LifeSpan);
}

UParticleSystem* AImpactEffect::GetImpactFX(TEnumAsByte<EPhysicalSurface> SurfaceType) const
{
	UParticleSystem* ImpactFX = NULL;
	if (bUseDefaultFX)
	{
		ImpactFX = DefaultFX;
	}

	//Look for it
	for (int i = 0; i < Impacts.Num(); i++)
	{
		if (Impacts[i].ImpactPhysicsMaterial == SurfaceType)
		{
			if (Impacts[i].ImpactFX)
			{
				ImpactFX = Impacts[i].ImpactFX;
				break;
			}
		}
	}
	return ImpactFX;
}

USoundCue* AImpactEffect::GetImpactSound(TEnumAsByte<EPhysicalSurface> SurfaceType) const
{
	USoundCue* ImpactSound = NULL;
	if (bUseDefaultFX)
	{
		ImpactSound = DefaultSound;
	}

	//Look for it
	for (int i = 0; i < Impacts.Num(); i++)
	{
		if (Impacts[i].ImpactPhysicsMaterial == SurfaceType)
		{
			if (Impacts[i].ImpactSound)
			{
				ImpactSound = Impacts[i].ImpactSound;
				break;
			}
		}
	}

	return ImpactSound;
}