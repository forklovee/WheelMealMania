// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/BaseDeliveryTargetArea.h"

#include "Game/BaseGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Vehicles/BaseVehicle.h"


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
}

void ABaseDeliveryTargetArea::PlayerVehicleStoppedInside_Implementation(ABaseVehicle* Vehicle)
{
	Super::PlayerVehicleStoppedInside_Implementation(Vehicle);
	
	AreaMesh->SetVisibility(false);

	ABaseGameMode* GameMode = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("%s GameMode is nullptr"), *GetName());
		return;
	}
	
	GameMode->DeliverActor(RequiredActor.Get(), this);
	AreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RequiredActor = nullptr;
}

void ABaseDeliveryTargetArea::BeginPlay()
{
	Super::BeginPlay();

	AreaMesh->SetVisibility(false);
}

bool ABaseDeliveryTargetArea::CanVehicleEnterArea(ABaseVehicle* Vehicle) const
{
	if (!RequiredActor.IsValid())
	{
		return false;
	}
	return Vehicle->HasPassenger(RequiredActor.Get());
}
