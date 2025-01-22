
#include "Objects/BasePickupArea.h"

#include "Game/BaseGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Objects/BaseDeliveryTargetArea.h"

// Sets default values
ABasePickupArea::ABasePickupArea()
{
	
}

void ABasePickupArea::SetDeliveryTarget(ABaseDeliveryTargetArea* InDeliveryTarget)
{
	DeliveryTargetArea = InDeliveryTarget;
	DeliveryTargetSet(DeliveryTargetArea.Get());
}

int ABasePickupArea::GetTimeSecondsToDeliveryTarget() const
{
	if (DeliveryTargetArea.IsValid())
	{
		const float DistanceToTarget = FVector::Distance(GetActorLocation(), DeliveryTargetArea->GetActorLocation());
		int TimeSeconds = 5;
		TimeSeconds += static_cast<int>(DistanceToTarget / 500.f);
		return TimeSeconds;
	}
	return 0;
}

void ABasePickupArea::BeginPlay()
{
	Super::BeginPlay();
}

void ABasePickupArea::OnVehicleStoppedInsideArea()
{
	Super::OnVehicleStoppedInsideArea();

	ABaseGameMode* GameMode = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GameMode)
	{
		return;
	}

	GameMode->AddDeliveryTarget(DeliveryTargetArea.Get());
	
	SecondsRemaining = GetTimeSecondsToDeliveryTarget();
	GetWorldTimerManager().SetTimer(DeliveryTickTimerHandle, this, &ABasePickupArea::OnDeliveryTimerTick, 1.0, true);
}

void ABasePickupArea::OnDeliveryTimerTick()
{
	SecondsRemaining--;
	DeliveryTimerUpdate(SecondsRemaining);
	if (SecondsRemaining <= 0)
	{
		DeliveryTickTimerHandle.Invalidate();
		DeliveryTimerStopped();
	}
}
