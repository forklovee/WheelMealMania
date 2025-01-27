// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/FareCharacter.h"

#include "Components/FareTimerComponent.h"
#include "Game/BaseGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Vehicles/BaseVehicle.h"
#include "Objects/BasePickupArea.h"
#include "Objects/BaseDeliveryTargetArea.h"

AFareCharacter::AFareCharacter()
{
	FareTimerComponent = CreateDefaultSubobject<UFareTimerComponent>(FName("FareTimer"));
}

ABasePickupArea* AFareCharacter::GetPickupArea_Implementation() const
{
	return PickupArea.Get();
}

ABaseDeliveryTargetArea* AFareCharacter::GetDeliveryTargetArea_Implementation() const
{
	return DeliveryTargetArea.Get();
}

int AFareCharacter::GetDeliveryTime_Implementation() const
{
	const float DistanceToTarget = FVector::Distance(PickupArea->GetActorLocation(), DeliveryTargetArea->GetActorLocation());
	return FareTimerComponent->GetMinDeliveryTime() + static_cast<int>(DistanceToTarget/2000.f);
}

int AFareCharacter::StartDelivery_Implementation(ABaseVehicle* Vehicle, ABasePickupArea* Start, ABaseDeliveryTargetArea* End)
{
	InVehicle = Vehicle;
	PickupArea = Start;
	DeliveryTargetArea = End;
	
	DeliveryTargetArea->SetRequiredActor(this);

	const int DeliveryTime = GetDeliveryTime_Implementation();
	
	FareTimerComponent->OnTimerTimeIsUp.AddUniqueDynamic(this, &AFareCharacter::DeliveryTimerTimeIsUp_Implementation);
	FareTimerComponent->StartTimer(DeliveryTime, DeliveryTargetArea.Get());
	
	DeliveryStarted_Implementation(this, GetDeliveryTimer_Implementation(), InVehicle.Get());
	
	return DeliveryTime;
}

int AFareCharacter::StopDelivery_Implementation(bool bSuccess)
{
	InVehicle = nullptr;
	PickupArea = nullptr;
	DeliveryTargetArea->SetRequiredActor(nullptr);
	DeliveryTargetArea = nullptr;
	
	const int RemainingDeliveryTime = GetDeliveryTimer_Implementation()->GetTimeRemaining();
	FareTimerComponent->OnTimerTimeIsUp.RemoveDynamic(this, &AFareCharacter::DeliveryTimerTimeIsUp_Implementation);

	if (bSuccess)
	{
		DeliveryFinished_Implementation(this, GetDeliveryTimer_Implementation(), InVehicle.Get());
	}
	else
	{
		DeliveryFailed_Implementation(this, GetDeliveryTimer_Implementation(), InVehicle.Get());
	}

	FareTimerComponent->StopTimer();
	
	return RemainingDeliveryTime;
}

void AFareCharacter::DeliveryTimerTimeIsUp_Implementation()
{
	IActorDeliveryInterface::DeliveryTimerTimeIsUp_Implementation();
	
	StopDelivery_Implementation(false);
}

void AFareCharacter::DeliveryStarted_Implementation(AActor* ActorToDeliver, UFareTimerComponent* FareTimerComponent, ABaseVehicle* Vehicle)
{
	ABaseGameMode* GameMode = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(this));
	GameMode->AddActorToDeliver(this);
}

void AFareCharacter::DeliveryFailed_Implementation(AActor* ActorToDeliver, UFareTimerComponent* FareTimerComponent, ABaseVehicle* Vehicle)
{
	ABaseGameMode* GameMode = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(this));
	GameMode->RemoveActorToDeliver(this);
}

void AFareCharacter::DeliveryFinished_Implementation(AActor* ActorToDeliver, UFareTimerComponent* FareTimerComponent, ABaseVehicle* Vehicle)
{
	ABaseGameMode* GameMode = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(this));
	GameMode->RemoveActorToDeliver(this);
	
	//Add Cash
	const float CashPerSecond = 2.5f;
	const int MaxDeliveryTime = FareTimerComponent->GetMaxDeliveryTime();
	const float CashEarned = MaxDeliveryTime * CashPerSecond;
	GameMode->AddCash(CashEarned);
}

UFareTimerComponent* AFareCharacter::GetDeliveryTimer_Implementation() const
{
	return FareTimerComponent;
}

ABaseVehicle* AFareCharacter::GetDeliveryVehicle_Implementation() const
{
	return InVehicle.Get();
}
