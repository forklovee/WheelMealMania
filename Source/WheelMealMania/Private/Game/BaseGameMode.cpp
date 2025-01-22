// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BaseGameMode.h"

#include "Blueprint/UserWidget.h"
#include "UI/PlayerHUD.h"
#include "Vehicles/BaseVehicle.h"

ABaseVehicle* ABaseGameMode::GetPlayerVehicle()
{
	if (PlayerVehicle == nullptr)
	{
		PlayerVehicle = Cast<ABaseVehicle>(GetWorld()->GetFirstPlayerController()->GetPawn());
	}
	return PlayerVehicle.Get();
}

TArray<ABaseDeliveryTargetArea*>& ABaseGameMode::GetDeliveryTargets()
{
	return DeliveryTargets;
}

void ABaseGameMode::AddDeliveryTarget(ABaseDeliveryTargetArea* NewDeliveryTargetArea)
{
	DeliveryTargets.Add(NewDeliveryTargetArea);

	OnNewDeliveryTargetAdded.Broadcast(NewDeliveryTargetArea);
	DeliveryTargetAdded(NewDeliveryTargetArea);
}

void ABaseGameMode::StartTimer(int TimeSeconds)
{
	TimeRemaining = TimeSeconds;

	GetWorld()->GetTimerManager().SetTimer(
		TimeoutTimerHandle,
		this, &ABaseGameMode::DecreaseTime,
		1.f, true, 1.f);

	OnTimerStarted.Broadcast();
	TimerStarted(TimeSeconds);
}

void ABaseGameMode::AddTime(int TimeSeconds)
{
	TimeRemaining += TimeSeconds;
	TimerTimeAdded(TimeSeconds);
}

int ABaseGameMode::GetRemainingTimeSeconds() const
{
	return TimeRemaining;
}

int ABaseGameMode::GetRemainingTimeMinutes() const
{
	return FMath::FloorToInt(TimeRemaining / 60.f);
}

void ABaseGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerVehicle = Cast<ABaseVehicle>(GetWorld()->GetFirstPlayerController()->GetPawn());
}

void ABaseGameMode::DecreaseTime()
{
	TimeRemaining -= 1;
	if (TimeRemaining <= 0) {
		StopTimer();
		TimeRemaining = 0;
		return;
	}

	OnTimerUpdated.Broadcast(
		GetRemainingTimeMinutes(),
		TimeRemaining - GetRemainingTimeMinutes() * 60
	);
	TimerUpdated(TimeRemaining);
}

void ABaseGameMode::StopTimer()
{
	TimeoutTimerHandle.Invalidate();
	OnTimerStopped.Broadcast();
	TimerStopped();
}
