// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NPCVehicleCrowdController.generated.h"

class APlayerVehicle;
class ANPCVehicle;

UCLASS()
class WHEELMEALMANIA_API ANPCVehicleCrowdController : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
	float ControllerTickInterval = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
	float VehicleDefaultTickInterval = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
	float VehicleFarTickInterval = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
	float FarRange = 2500.f;
	
private:
	TArray<ANPCVehicle*> Vehicles;
	
	TWeakObjectPtr<APlayerCameraManager> PlayerCamera;
	
public:	
	ANPCVehicleCrowdController();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	bool IsVehicleOutOfFarRange(ANPCVehicle* Vehicle) const;
	bool IsVehicleVisible(ANPCVehicle* Vehicle) const;
	
};
