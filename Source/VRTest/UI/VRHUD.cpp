// Fill out your copyright notice in the Description page of Project Settings.

#include "VRHUD.h"
#include "Engine/Canvas.h"
#include "Firearm.h"
#include "Magazine.h"
#include "Hand.h"
#include "PlayerPawn.h"

void AVRHUD::DrawHUD()
{
	Super::DrawHUD();

	auto PlayerPawn = Cast<APlayerPawn>(GetOwningPawn());
	if(!PlayerPawn)
	{
		return;
	}

	TArray<AHand*>	Hands;
	Hands.Add(PlayerPawn->LeftHand);
	Hands.Add(PlayerPawn->RightHand);

	for (auto Hand : Hands)
	{
		auto HeldFirearm = Cast<AFirearm>(Hand->GetInteractingActor());
		if (HeldFirearm)
		{
			auto HeldMagazine = HeldFirearm->GetLoadedMagazine();
			if (HeldMagazine)
			{
				FString MagazineAmmoText = FString::Printf(TEXT("%d / %d"), HeldMagazine->CurrentAmmo, HeldMagazine->AmmoCount);
				FVector MagLocation = Canvas->Project(HeldMagazine->GetActorLocation());

				//DrawText(MagazineAmmoText, FColor::White, MagLocation.X, MagLocation.Y, nullptr, 4.0f);
			}
		}

		auto HeldMagazine = Cast<AMagazine>(Hand->GetInteractingActor());
		if (HeldMagazine)
		{
			FString MagazineAmmoText = FString::Printf(TEXT("%d / %d"), HeldMagazine->CurrentAmmo, HeldMagazine->AmmoCount);
			FVector MagLocation = Canvas->Project(HeldMagazine->GetActorLocation());

			//DrawText(MagazineAmmoText, FColor::White, MagLocation.X, MagLocation.Y, nullptr, 4.0f);
		}

		for (auto NearbyActor : Hand->GetNearbyActors())
		{
			if (!Hand->GetInteractingActor())
			{

				Draw3DLine(NearbyActor->GetActorLocation(), Hand->GetHandSelectionOrigin(), FColor::Green);
			}
		}
	}
}
