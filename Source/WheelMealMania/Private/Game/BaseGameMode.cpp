// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BaseGameMode.h"

#include "Components/FareTimerComponent.h"
#include "Interfaces/ActorDeliveryInterface.h"
#include "Objects/BaseDeliveryTargetArea.h"
#include "Vehicles/BaseVehicle.h"

ABaseVehicle* ABaseGameMode::GetPlayerVehicle() const
{
	return Cast<ABaseVehicle>(GetWorld()->GetFirstPlayerController()->GetPawn());
}

TArray<AActor*>& ABaseGameMode::GetActorsToDeliver()
{
	return ActorsToDeliver;
}

void ABaseGameMode::AddActorToDeliver(AActor* ActorToDeliver)
{
	if (!ActorToDeliver->Implements<UActorDeliveryInterface>())
	{
		UE_LOG(LogTemp, Error, TEXT("%s Actor doesn't implement IActorDeliveryInterface!"), *ActorToDeliver->GetName());
		return;
	}
	ActorsToDeliver.Add(ActorToDeliver);
	
	const int DeliveryTime = IActorDeliveryInterface::Execute_GetDeliveryTime(ActorToDeliver);
	if (TimeRemaining < DeliveryTime)
	{
		AddTime(DeliveryTime - TimeRemaining);
	}
	UE_LOG(LogTemp, Display, TEXT("Delivery Time: %i"), DeliveryTime);

	UFareTimerComponent* FareTimerComponent = IActorDeliveryInterface::Execute_GetDeliveryTimer(ActorToDeliver);
	OnActorToDeliverAdded.Broadcast(ActorToDeliver, FareTimerComponent);
	ActorToDeliverAdded(ActorToDeliver, FareTimerComponent);
}

void ABaseGameMode::RemoveActorToDeliver(AActor* ActorToDeliver)
{
	if (!ActorToDeliver->Implements<UActorDeliveryInterface>())
	{
		UE_LOG(LogTemp, Error, TEXT("%s Actor doesn't implement IActorDeliveryInterface!"), *ActorToDeliver->GetName());
		return;
	}
	ActorsToDeliver.Remove(ActorToDeliver);
	
	UFareTimerComponent* FareTimerComponent = IActorDeliveryInterface::Execute_GetDeliveryTimer(ActorToDeliver);
	OnActorDelivered.Broadcast(ActorToDeliver, FareTimerComponent);

	Clients++;
	
	//Add Bonus Time
	const int TimeRemaining = FareTimerComponent->GetTimeRemaining();
	if (TimeRemaining > 0)
	{
		AddTime((int)(TimeRemaining*.75f));
		OnBonusTimeAdded.Broadcast(TimeRemaining);
	}
	ActorDelivered(ActorToDeliver, FareTimerComponent);
}

void ABaseGameMode::FailActorDelivery(AActor* ActorToDeliver, UFareTimerComponent* FareTimerComponent)
{
	ActorsToDeliver.Remove(ActorToDeliver);
	OnActorDeliveryFailed.Broadcast(ActorToDeliver, FareTimerComponent);
	ActorDeliveryFailed(ActorToDeliver, FareTimerComponent);
}

void ABaseGameMode::AddCash(float CashEarned)
{
	Cash += CashEarned;
	UE_LOG(LogTemp, Display, TEXT("Cash: %f +%f"), Cash, CashEarned);
	OnCashAdded.Broadcast(Cash, CashEarned);
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
	const int OldTimeRemaining = TimeRemaining;
	TimeRemaining += TimeSeconds;
	TimerTimeAdded(OldTimeRemaining, TimeSeconds);
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
}

void ABaseGameMode::DecreaseTime()
{
	TimeRemaining -= 1;
	if (TimeRemaining <= 0) {
		OnTimerTimesUp.Broadcast();
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
