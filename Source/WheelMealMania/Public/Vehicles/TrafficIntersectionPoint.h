// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Vehicles/TrafficVehicleGuidePoint.h"
#include "TrafficIntersectionPoint.generated.h"


class AIntersectionController;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWaitStateChangedSignature, ATrafficIntersectionPoint*,
                                             TrafficIntersectionPoint, bool, bIsWaiting);

UCLASS()
class WHEELMEALMANIA_API ATrafficIntersectionPoint : public ATrafficVehicleGuidePoint
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FWaitStateChangedSignature WaitStateChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	bool bDisabled = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	bool bIsRightLane = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	bool bIsWaiting = true;

private:
	TWeakObjectPtr<AIntersectionController> IntersectionController;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Traffic")
	bool IsDisabled() const;
	
	UFUNCTION(BlueprintCallable, Category = "Traffic")
	AIntersectionController* GetIntersectionController() const;
	UFUNCTION(BlueprintCallable, Category = "Traffic")
	virtual void SetIntersectionController(AIntersectionController* NewIntersectionController);
	
	UFUNCTION(BlueprintCallable, Category = "Traffic")
	bool GetWaitingState() const;
	UFUNCTION(BlueprintCallable, Category = "Traffic")
	virtual void SetWaitingState(bool bNewIsWaiting);
	UFUNCTION(BlueprintImplementableEvent)
	void OnWaitingStateChanged(bool bNewWaitingState);
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	
};
