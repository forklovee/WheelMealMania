// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/FareTimerComponent.h"
#include "Objects/BaseDeliveryTargetArea.h"

UFareTimerComponent::UFareTimerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFareTimerComponent::StartTimer(const int TimeSeconds, ABaseDeliveryTargetArea* NewDeliveryTarget)
{
	DeliveryTarget = NewDeliveryTarget;
	
	MaxDeliveryTime = TimeSeconds;
	TimeRemaining = TimeSeconds;
	OnTimerStarted.Broadcast(TimeSeconds);
	
	GetWorld()->GetTimerManager().SetTimer(TimerHandle,
		this,
		&UFareTimerComponent::UpdateTimer,
		1.f,
		true);
}

void UFareTimerComponent::StopTimer()
{
	OnTimerStopped.Broadcast(TimeRemaining);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	TimeRemaining = 0;
}

int UFareTimerComponent::GetTimeRemaining() const
{
	return TimeRemaining;
}

int UFareTimerComponent::GetMaxDeliveryTime() const
{
	return MaxDeliveryTime;
}

int UFareTimerComponent::GetMinDeliveryTime() const
{
	return MinDeliveryTime;
}

ABaseDeliveryTargetArea* UFareTimerComponent::GetDeliveryTarget() const
{
	return DeliveryTarget.Get();
}

void UFareTimerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UFareTimerComponent::UpdateTimer()
{
	TimeRemaining--;
	OnTimerUpdated.Broadcast(TimeRemaining);
	if (TimeRemaining <= 0)
	{
		OnTimerTimeIsUp.Broadcast();
		StopTimer();
	}
}
