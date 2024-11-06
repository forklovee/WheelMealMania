// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerHUD.h"

void UPlayerHUD::UpdateSpeed(float NewSpeed)
{
	OnSpeedUpdate(NewSpeed);
}
