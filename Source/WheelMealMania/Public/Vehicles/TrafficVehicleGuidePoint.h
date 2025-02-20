// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrafficVehicleGuidePoint.generated.h"

UCLASS()
class WHEELMEALMANIA_API ATrafficVehicleGuidePoint : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	bool bIsEnabled = true;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* DebugMesh;
	
public:	
	ATrafficVehicleGuidePoint();

protected:
	virtual void BeginPlay() override;

};
