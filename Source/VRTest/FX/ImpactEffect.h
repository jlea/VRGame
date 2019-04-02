// Copyright 2010-2016 New World Interactive LLC and Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GameFramework/Actor.h"
#include "ImpactEffect.generated.h"

class USoundCue;
class UParticleSystem;

UENUM()
namespace EPhysMaterialType
{
	enum Type
	{
		Unknown, // = SurfaceType_Default
		Concrete, // = SurfaceType1
		Dirt, // = SurfaceType2
		Water, // = SurfaceType3
		Metal, // = SurfaceType4
		Wood, // = SurfaceType5
		Grass, // = SurfaceType6
		Glass, // = SurfaceType7
		Flesh, // = SurfaceType8
		Leaves, // = SurfaceType9
		Sand, // = SurfaceType10
		Snow, // = SurfaceType11
		Tile, // = SurfaceType12
		Cloth, // = SurfaceType13
		Carpet, // = SurfaceType14
		Brick, // = SurfaceType15
		Plaster, // = SurfaceType16
		Electronics, // = SurfaceType17
		Plastic, // = SurfaceType18
	};
}

USTRUCT()
struct FDecalData
{
	GENERATED_USTRUCT_BODY()

	/** Material to show */
	UPROPERTY(EditDefaultsOnly, Category = Decal)
	UMaterial* DecalMaterial;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category = Decal)
		float DecalSize;

	/** lifespan before dissapearing */
	UPROPERTY(EditDefaultsOnly, Category = Decal)
		float LifeSpan;

	/** defaults */
	FDecalData()
		: DecalSize(4.0f)
		, LifeSpan(120.0f)
	{
	}
};


/* Holds the data for a certain surface type */
USTRUCT()
struct FImpactData
{
	GENERATED_USTRUCT_BODY()

	/** Sound to play on this surface */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* ImpactSound;

	/** FX to spawn on this surface */
	UPROPERTY(EditDefaultsOnly, Category = Particle)
	UParticleSystem* ImpactFX;

	/* Surface we are associated wtih */
	UPROPERTY(EditDefaultsOnly, Category = Physics)
	TEnumAsByte<EPhysMaterialType::Type> ImpactPhysicsMaterial;

	/** defaults */
	FImpactData()
	{
		ImpactSound = NULL;
		ImpactFX = NULL;
		ImpactPhysicsMaterial = EPhysMaterialType::Concrete;
	}
};

UCLASS(config = Game, BlueprintType)
class AImpactEffect : public AActor
{
	GENERATED_UCLASS_BODY()

	virtual void BeginPlay() override;

	//////////////////////////////////////////////////////////////////////////
	// Functions

	/** get FX for designated material type */
	UParticleSystem* GetImpactFX(TEnumAsByte<EPhysicalSurface> SurfaceType) const;

	/** get sound for designated material type */
	USoundCue* GetImpactSound(TEnumAsByte<EPhysicalSurface> SurfaceType) const;

	//////////////////////////////////////////////////////////////////////////
	// Properties

	/** Default FX if there's no override for a surface */
	UPROPERTY(EditDefaultsOnly, Category = Defaults)
	UParticleSystem* DefaultFX;

	/** Default sound if there's no override for a surface */
	UPROPERTY(EditDefaultsOnly, Category = Defaults)
	USoundCue* DefaultSound;

	/** Default decal if there's no override for a surface */
	UPROPERTY(EditDefaultsOnly, Category = Defaults)
	struct FDecalData DefaultDecal;

	/* All of the surfaces we have effects for */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
	TArray<FImpactData> Impacts;

	/** Pass this in during construction so the actor knows what surface it hit */
	UPROPERTY(BlueprintReadOnly, Category = Surface)
	FHitResult SurfaceHit;

	/** Use default FX if there's no corresponding FX */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
	bool bUseDefaultFX = true;
};
