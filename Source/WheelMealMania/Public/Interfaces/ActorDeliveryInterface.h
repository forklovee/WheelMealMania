// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/FareTimerComponent.h"
#include "UObject/Interface.h"
#include "ActorDeliveryInterface.generated.h"

class UFareTimerComponent;
class APlayerVehicle;
class UDeliveryTimer;
class ABaseDeliveryTargetArea;
class ABasePickupArea;

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
	//Returns Delivery Time
	int StartDelivery(APlayerVehicle* Vehicle, ABasePickupArea* Start, ABaseDeliveryTargetArea* End);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	//Returns Remaining Delivery Time
	int StopDelivery(bool bSuccess = false);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	ABasePickupArea* GetPickupArea() const;
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	ABaseDeliveryTargetArea* GetDeliveryTargetArea() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	int GetDeliveryTime() const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UFareTimerComponent* GetDeliveryTimer() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	APlayerVehicle* GetDeliveryVehicle() const;

protected:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DeliveryStarted(AActor* ActorToDeliver, UFareTimerComponent* FareTimerComponent, APlayerVehicle* Vehicle);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DeliveryTimerTimeIsUp();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DeliveryFailed(AActor* ActorToDeliver, UFareTimerComponent* FareTimerComponent, APlayerVehicle* Vehicle);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DeliveryFinished(AActor* ActorToDeliver, UFareTimerComponent* FareTimerComponent, APlayerVehicle* Vehicle);
};
