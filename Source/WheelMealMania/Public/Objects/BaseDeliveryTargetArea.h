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
	
public:	
	ABaseDeliveryTargetArea();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
