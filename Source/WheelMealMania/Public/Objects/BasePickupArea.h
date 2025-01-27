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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Delivery")
	bool bSpawnActorOnBeginPlay = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Delivery")
	TSubclassOf<AActor> ActorToDeliverClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Area")
	TWeakObjectPtr<ABaseDeliveryTargetArea> DeliveryTargetArea;
	
private:
	FTimerHandle DeliveryTickTimerHandle;
	int SecondsRemaining = 0;

	TWeakObjectPtr<AActor> ActorToDeliver;
	
public:	
	ABasePickupArea();

	UFUNCTION(BlueprintCallable)
	void SpawnActorToDeliver();

	UFUNCTION(BlueprintCallable)
	AActor* GetActorToDeliver() const;

protected:
	virtual void BeginPlay() override;
	
	virtual void PlayerVehicleStoppedInside_Implementation(ABaseVehicle* Vehicle) override;
};
