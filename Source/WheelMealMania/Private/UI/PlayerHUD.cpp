// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerHUD.h"

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