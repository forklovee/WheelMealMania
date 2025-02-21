// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/FareCharacter.h"

#include "Components/FareTimerComponent.h"
#include "Game/BaseGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Vehicles/PlayerVehicle.h"
#include "Objects/BasePickupArea.h"
#include "Objects/BaseDeliveryTargetArea.h"

AFareCharacter::AFareCharacter()
{
	FareTimerComponent = CreateDefaultSubobject<UFareTimerComponent>(FName("FareTimer"));
}

void AFareCharacter::BeginPlay()
{
	Super::BeginPlay();
	// GetMesh()->SetSimulatePhysics(false);
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
	return FareTimerComponent->GetMinDeliveryTime() + static_cast<int>(DistanceToTarget/2800.f);
}

int AFareCharacter::StartDelivery_Implementation(APlayerVehicle* Vehicle, ABasePickupArea* Start, ABaseDeliveryTargetArea* End)
{
	InVehicle = Vehicle;
	PickupArea = Start;
	DeliveryTargetArea = End;

	if (!DeliveryTargetArea.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("%s: Delivery Target Not Set."), *GetName());
		return 0;
	}
	
	DeliveryTargetArea->SetRequiredActor(this);

	const int DeliveryTime = Execute_GetDeliveryTime(this);
	
	FareTimerComponent->OnTimerTimeIsUp.AddDynamic(this, &AFareCharacter::FareTimerTimeIsUp);
	FareTimerComponent->StartTimer(DeliveryTime, DeliveryTargetArea.Get());
	
	Execute_DeliveryStarted(this, this, Execute_GetDeliveryTimer(this), InVehicle.Get());
	
	return DeliveryTime;
}

int AFareCharacter::StopDelivery_Implementation(bool bSuccess)
{
	DeliveryTargetArea->SetRequiredActor(nullptr);
	const int RemainingDeliveryTime = Execute_GetDeliveryTimer(this)->GetTimeRemaining();
	FareTimerComponent->OnTimerTimeIsUp.RemoveDynamic(this, &AFareCharacter::FareTimerTimeIsUp);
	
	if (bSuccess)
	{
		Execute_DeliveryFinished(this, this, Execute_GetDeliveryTimer(this), InVehicle.Get());
	}
	else
	{
		Execute_DeliveryFailed(this, this, Execute_GetDeliveryTimer(this), InVehicle.Get());
	}
	
	InVehicle = nullptr;
	PickupArea = nullptr;
	DeliveryTargetArea = nullptr;
	
	FareTimerComponent->StopTimer();
	return RemainingDeliveryTime;
}

void AFareCharacter::DeliveryTimerTimeIsUp_Implementation()
{
	IActorDeliveryInterface::DeliveryTimerTimeIsUp_Implementation();
	
	Execute_StopDelivery(this, false);
}

void AFareCharacter::FareTimerTimeIsUp()
{
	Execute_DeliveryTimerTimeIsUp(this);
}

void AFareCharacter::DeliveryStarted_Implementation(AActor* ActorToDeliver, UFareTimerComponent* FareTimerComponent, APlayerVehicle* Vehicle)
{
	IActorDeliveryInterface::DeliveryStarted_Implementation(ActorToDeliver, FareTimerComponent, Vehicle);
	
	ABaseGameMode* GameMode = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(this));
	GameMode->AddActorToDeliver(this);

	UE_LOG(LogTemp, Warning, TEXT("Delivery Started! %s"), *GetName());
}

void AFareCharacter::DeliveryFailed_Implementation(AActor* ActorToDeliver, UFareTimerComponent* FareTimerComponent, APlayerVehicle* Vehicle)
{
	IActorDeliveryInterface::DeliveryFailed_Implementation(ActorToDeliver, FareTimerComponent, Vehicle);
	
	ABaseGameMode* GameMode = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(this));
	GameMode->RemoveActorToDeliver(this);

	UE_LOG(LogTemp, Warning, TEXT("Delivery Failed! %s"), *GetName());
}

void AFareCharacter::DeliveryFinished_Implementation(AActor* ActorToDeliver, UFareTimerComponent* FareTimerComponent, APlayerVehicle* Vehicle)
{
	IActorDeliveryInterface::DeliveryFinished_Implementation(ActorToDeliver, FareTimerComponent, Vehicle);

	ABaseGameMode* GameMode = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(this));
	GameMode->RemoveActorToDeliver(this);
	
	//Add Cash
	const float CashPerSecond = 2.5f;
	const int MaxDeliveryTime = FareTimerComponent->GetMaxDeliveryTime();
	const float CashEarned = MaxDeliveryTime * CashPerSecond;
	GameMode->AddCash(CashEarned);

	UE_LOG(LogTemp, Warning, TEXT("Delivery Finished! %s"), *GetName());
}

UFareTimerComponent* AFareCharacter::GetDeliveryTimer_Implementation() const
{
	return FareTimerComponent;
}

APlayerVehicle* AFareCharacter::GetDeliveryVehicle_Implementation() const
{
	return InVehicle.Get();
}