// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BaseGameMode.h"

#include "Components/FareTimerComponent.h"
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

void ABaseGameMode::AddActorToDeliver(AActor* ActorToDeliver, ABaseDeliveryTargetArea* DeliveryTargetArea)
{
	UActorComponent* ActorFareTimerComponent = ActorToDeliver->FindComponentByClass(UFareTimerComponent::StaticClass());
	UFareTimerComponent* FareTimerComponent = Cast<UFareTimerComponent>(ActorFareTimerComponent);
	if (FareTimerComponent == nullptr)
	{
		return;
	}
	ActorsToDeliver.Add(ActorToDeliver);
	DeliveryTargetArea->SetRequiredActor(ActorToDeliver);
	
	const float DistanceToTarget = FVector::Distance(ActorToDeliver->GetActorLocation(), DeliveryTargetArea->GetActorLocation());
	const int DeliveryTime = FareTimerComponent->GetMinDeliveryTime() + static_cast<int>(DistanceToTarget/2000.f);
	FareTimerComponent->StartTimer(DeliveryTime, DeliveryTargetArea);

	if (DeliveryTime < TimeRemaining)
	{
		AddTime(TimeRemaining-DeliveryTime);
	}
	UE_LOG(LogTemp, Display, TEXT("Delivery Time: %i"), DeliveryTime);

	OnActorToDeliverAdded.Broadcast(ActorToDeliver, FareTimerComponent);
	ActorToDeliverAdded(ActorToDeliver, FareTimerComponent);
}

void ABaseGameMode::DeliverActor(AActor* ActorToDeliver, ABaseDeliveryTargetArea* DeliveryTargetArea)
{
	UActorComponent* ActorFareTimerComponent = ActorToDeliver->FindComponentByClass(UFareTimerComponent::StaticClass());
	UFareTimerComponent* FareTimerComponent = Cast<UFareTimerComponent>(ActorFareTimerComponent);
	if (FareTimerComponent == nullptr)
	{
		return;
	}
	ActorsToDeliver.Remove(ActorToDeliver);
	
	//Add Bonus Time
	const int TimeRemaining = FareTimerComponent->GetTimeRemaining();
	if (TimeRemaining > 0)
	{
		AddTime(TimeRemaining);
		OnBonusTimeAdded.Broadcast(TimeRemaining);
	}

	//Add Cash
	const int MaxDeliveryTime = FareTimerComponent->GetMaxDeliveryTime();
	const float CashEarned = MaxDeliveryTime * CashPerSecond;
	Cash += CashEarned;
	UE_LOG(LogTemp, Display, TEXT("Cash: %f +%f"), Cash, CashEarned);
	OnCashAdded.Broadcast(Cash, CashEarned);
	
	FareTimerComponent->StopTimer();
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
