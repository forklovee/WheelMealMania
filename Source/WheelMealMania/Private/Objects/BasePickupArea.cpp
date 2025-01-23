
#include "Objects/BasePickupArea.h"

#include "Game/BaseGameMode.h"
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
	
	ActorToDeliver = GetWorld()->SpawnActor<AActor>(ActorToDeliverClass,
		GetActorLocation() + FVector::UpVector*100.f,
		FRotator::ZeroRotator
		);
	
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

void ABasePickupArea::OnVehicleStoppedInsideArea()
{
	Super::OnVehicleStoppedInsideArea();

	ABaseGameMode* GameMode = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("%s GameMode is nullptr"), *GetName());
		return;
	}
	GameMode->AddActorToDeliver(ActorToDeliver.Get(), DeliveryTargetArea.Get());
	AreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}