// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/BaseDeliveryTargetArea.h"

#include "Game/BaseGameMode.h"
#include "Interfaces/ActorDeliveryInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Vehicles/BaseVehicle.h"
#include "Vehicles/PlayerVehicle.h"


ABaseDeliveryTargetArea::ABaseDeliveryTargetArea()
{
}

AActor* ABaseDeliveryTargetArea::GetRequiredActor() const
{
	return RequiredActor.Get();
}

void ABaseDeliveryTargetArea::SetRequiredActor(AActor* PassengerActor)
{
	RequiredActor = PassengerActor;
	AreaMesh->SetVisibility(RequiredActor.IsValid());
	AreaMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
}

void ABaseDeliveryTargetArea::PlayerVehicleStoppedInside_Implementation(APlayerVehicle* Vehicle)
{
	Super::PlayerVehicleStoppedInside_Implementation(Vehicle);

	if (!RequiredActor.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Delivery Target: Required Actor Invalid."));
		return;
	}
	
	if (!RequiredActor->Implements<UActorDeliveryInterface>())
	{
		return;
	}
	
	IActorDeliveryInterface::Execute_StopDelivery(RequiredActor.Get(), true);
	
	AreaMesh->SetVisibility(false);
	
	AreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RequiredActor = nullptr;
}

void ABaseDeliveryTargetArea::BeginPlay()
{
	Super::BeginPlay();

	AreaMesh->SetVisibility(false);
}

bool ABaseDeliveryTargetArea::CanVehicleEnterArea(APlayerVehicle* Vehicle) const
{
	if (!RequiredActor.IsValid())
	{
		return false;
	}
	return Vehicle->HasPassenger(RequiredActor.Get());
}
