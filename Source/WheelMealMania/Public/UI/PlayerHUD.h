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

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnSpeedUpdate(float NewSpeed);
};
