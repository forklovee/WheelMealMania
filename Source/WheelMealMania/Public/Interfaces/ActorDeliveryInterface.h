// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ActorDeliveryInterface.generated.h"

class ABaseVehicle;
class UDeliveryTimer;
class ABaseDeliveryTargetArea;
class ABasePickupArea;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UActorDeliveryInterface : public UInterface
{
	GENERATED_BODY()
};

class WHEELMEALMANIA_API IActorDeliveryInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetStartAndEndPoints(ABasePickupArea* Start, ABaseDeliveryTargetArea* End);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	ABasePickupArea* GetPickupArea() const;
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	ABaseDeliveryTargetArea* GetTargetArea() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void StartDelivery(ABaseVehicle* Vehicle);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DeliveryStarted(ABaseVehicle* Vehicle);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DeliveryFailed(ABaseVehicle* Vehicle);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DeliveryFinished(ABaseVehicle* Vehicle);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UDeliveryTimer* GetDeliveryTimer() const;
};
