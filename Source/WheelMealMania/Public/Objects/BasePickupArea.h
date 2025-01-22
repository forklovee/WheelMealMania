// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VehicleEventArea.h"
#include "GameFramework/Actor.h"
#include "BasePickupArea.generated.h"

class ABaseDeliveryTargetArea;

UCLASS()
class ABasePickupArea : public AVehicleEventArea
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Area")
	TWeakObjectPtr<ABaseDeliveryTargetArea> DeliveryTargetArea;

private:
	FTimerHandle DeliveryTickTimerHandle;
	int SecondsRemaining = 0;
	
public:	
	ABasePickupArea();

	void SetDeliveryTarget(ABaseDeliveryTargetArea* NewDeliveryTarget);

	UFUNCTION(BlueprintCallable, Category = "Pickup Area")
	int GetTimeSecondsToDeliveryTarget() const;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
	void DeliveryTargetSet(ABaseDeliveryTargetArea* NewDeliveryTarget);

	UFUNCTION()
	virtual void OnVehicleStoppedInsideArea() override;

	UFUNCTION(BlueprintImplementableEvent)
	void DeliveryTimerUpdate(int SecondsRemaining);

	UFUNCTION(BlueprintImplementableEvent)
	void DeliveryTimerStopped();
private:
	UFUNCTION()
	void OnDeliveryTimerTick();
	void OnDeliveryTimerEnded();
};
