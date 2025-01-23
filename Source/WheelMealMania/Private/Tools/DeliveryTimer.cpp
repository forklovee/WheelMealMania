// Fill out your copyright notice in the Description page of Project Settings.

#include "Tools/DeliveryTimer.h"

void UDeliveryTimer::StartTimer(const int DeliveryTimeSeconds)
{
	TimeRemaining = DeliveryTimeSeconds;
	GetWorld()->GetTimerManager().SetTimer(Timer, this, &UDeliveryTimer::UpdateTimer, 1.0, true);
}

void UDeliveryTimer::StopTimer()
{
	OnTimerStopped.Broadcast(TimeRemaining);
	GetWorld()->GetTimerManager().ClearTimer(Timer);
	TimeRemaining = 0;
}

int UDeliveryTimer::GetTimeRemaining() const
{
	return TimeRemaining;
}

void UDeliveryTimer::UpdateTimer()
{
	TimeRemaining--;
	OnTimerUpdated.Broadcast(TimeRemaining);

	if (TimeRemaining <= 0)
	{
		StopTimer();
	}
}
