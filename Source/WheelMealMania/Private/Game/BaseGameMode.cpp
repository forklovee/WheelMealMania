// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BaseGameMode.h"

void ABaseGameMode::StartTimer(int TimeSeconds)
{
	TimeRemaining = TimeSeconds;

	GetWorld()->GetTimerManager().SetTimer(
		TimeoutTimerHandle,
		this, &ABaseGameMode::DecreaseTime,
		1.f, true, 1.f);

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

void ABaseGameMode::DecreaseTime()
{
	TimeRemaining -= 1;
	if (TimeRemaining <= 0) {
		StopTimer();
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
	TimerStopped();
}
