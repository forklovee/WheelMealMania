
#include "Objects/BasePickupArea.h"

#include "Game/BaseGameMode.h"
#include "Interfaces/ActorDeliveryInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Objects/BaseDeliveryTargetArea.h"

ABasePickupArea::ABasePickupArea()
{
	
}

void ABasePickupArea::SpawnActorToDeliver()
{
	if (ActorToDeliver.IsValid())
	{
		UE_LOG(LogTemp, Display, TEXT("%s Actor already spawned"), *GetName());
		return;
	}
	if (ActorToDeliverClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s Actor to deliver class nullptr"), *GetName());
		return;
	}

	FHitResult Hit;
	UKismetSystemLibrary::LineTraceSingle(
		this,
		GetActorLocation() + FVector::UpVector * 5000.f,
		GetActorLocation() - FVector::UpVector * 5000.f,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		{},
		EDrawDebugTrace::None,
		Hit,
		true);

	FVector SpawnLocation = GetActorLocation() + FVector::UpVector*100.f;
	if (Hit.bBlockingHit)
	{
		SpawnLocation = Hit.Location + FVector::UpVector * 150.f;
	}
	
	ActorToDeliver = GetWorld()->SpawnActor<AActor>(ActorToDeliverClass, SpawnLocation, FRotator::ZeroRotator);
	
	APawn* Pawn = Cast<APawn>(ActorToDeliver.Get());
	if (Pawn)
	{
		Pawn->SpawnDefaultController();
	}
}

AActor* ABasePickupArea::GetActorToDeliver() const
{
	return ActorToDeliver.Get();
}

void ABasePickupArea::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnActorOnBeginPlay)
	{
		SpawnActorToDeliver();
	}
}

void ABasePickupArea::PlayerVehicleStoppedInside_Implementation(APlayerVehicle* Vehicle)
{
	Super::PlayerVehicleStoppedInside_Implementation(Vehicle);

	if (!ActorToDeliver->Implements<UActorDeliveryInterface>())
	{
		return;
	}

	IActorDeliveryInterface::Execute_StartDelivery(ActorToDeliver.Get(), Vehicle, this, DeliveryTargetArea.Get());
	
	AreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AreaMesh->SetVisibility(false);
}

