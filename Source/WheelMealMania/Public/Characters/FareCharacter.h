// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Interfaces/ActorDeliveryInterface.h"
#include "FareCharacter.generated.h"

/**
 * 
 */

class ABasePickupArea;
class ABaseDeliveryTargetArea;
class UFareTimerComponent;

UCLASS()
class WHEELMEALMANIA_API AFareCharacter : public ABaseCharacter, public IActorDeliveryInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Components", meta=(AllowPrivateAccess=true))
	UFareTimerComponent* FareTimerComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UBehaviorTree> FareBehaviorTree;
	
private:
	TWeakObjectPtr<ABasePickupArea> PickupArea;
	TWeakObjectPtr<ABaseDeliveryTargetArea> DeliveryTargetArea;

	TWeakObjectPtr<APlayerVehicle> InVehicle;
	
public:
	AFareCharacter();

	//Delivery
	virtual int StartDelivery_Implementation(APlayerVehicle* Vehicle, ABasePickupArea* Start, ABaseDeliveryTargetArea* End) override;
	virtual int StopDelivery_Implementation(bool bSuccess = false) override;

	//Getters
	virtual ABasePickupArea* GetPickupArea_Implementation() const override;
	virtual ABaseDeliveryTargetArea* GetDeliveryTargetArea_Implementation() const override;
	virtual int GetDeliveryTime_Implementation() const override;
	virtual UFareTimerComponent* GetDeliveryTimer_Implementation() const override;
	virtual APlayerVehicle* GetDeliveryVehicle_Implementation() const override;
	
protected:
	virtual void BeginPlay() override;
	
	//Events
	virtual void DeliveryStarted_Implementation(AActor* ActorToDeliver, UFareTimerComponent* FareTimerComponent, APlayerVehicle* Vehicle) override;
	virtual void DeliveryFailed_Implementation(AActor* ActorToDeliver, UFareTimerComponent* FareTimerComponent, APlayerVehicle* Vehicle) override;
	virtual void DeliveryFinished_Implementation(AActor* ActorToDeliver, UFareTimerComponent* FareTimerComponent, APlayerVehicle* Vehicle) override;

	UFUNCTION()
	virtual void DeliveryTimerTimeIsUp_Implementation() override;

private:
	UFUNCTION()
	void FareTimerTimeIsUp();
};