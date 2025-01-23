// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VehicleEventArea.h"
#include "GameFramework/Actor.h"
#include "BaseDeliveryTargetArea.generated.h"

UCLASS()
class WHEELMEALMANIA_API ABaseDeliveryTargetArea : public AVehicleEventArea
{
	GENERATED_BODY()

private:
	TWeakObjectPtr<AActor> RequiredActor;
	
public:	
	ABaseDeliveryTargetArea();
	
	UFUNCTION(BlueprintCallable)
	AActor* GetRequiredActor() const;
	UFUNCTION(BlueprintCallable)
	void SetRequiredActor(AActor* PassengerActor);

	virtual void PlayerVehicleStoppedInside_Implementation(ABaseVehicle* Vehicle) override;
	
protected:
	virtual void BeginPlay() override;

	virtual bool CanVehicleEnterArea(ABaseVehicle* Vehicle) const override;

};
