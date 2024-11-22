// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */
UCLASS()
class WHEELMEALMANIA_API UPlayerHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateSpeed(float NewSpeed);

	void UpdateCurrentGear(EGearShift CurrentGear);
	void UpdateIsGearShifting(bool bIsGearShifting);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnSpeedUpdate(float NewSpeed);

	UFUNCTION(BlueprintImplementableEvent)
	void OnCurrentGearUpdate(EGearShift CurrentGear);
	UFUNCTION(BlueprintImplementableEvent)
	void OnIsGearShiftingUpdate(bool bIsGearShifting);

};
