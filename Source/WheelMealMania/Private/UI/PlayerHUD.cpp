// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/PlayerHUD.h"
#include "Vehicles/BaseVehicle.h"

void UPlayerHUD::SetPlayerVehicle(ABaseVehicle* NewPlayerVehicle)
{
	PlayerVehicle = NewPlayerVehicle;
}

void UPlayerHUD::UpdateSpeed(float NewSpeed)
{
	OnSpeedUpdate(NewSpeed);
}

void UPlayerHUD::UpdateCurrentGear(EGearShift CurrentGear)
{
	OnCurrentGearUpdate(CurrentGear);
}

void UPlayerHUD::UpdateIsGearShifting(bool bIsGearShifting)
{
	OnIsGearShiftingUpdate(bIsGearShifting);
}

void UPlayerHUD::PlayerPerformedTrick(EVehicleTrick NewTrick, int TricksPerformed)
{
	OnPlayerPerformedTrick(NewTrick, TricksPerformed);
}
