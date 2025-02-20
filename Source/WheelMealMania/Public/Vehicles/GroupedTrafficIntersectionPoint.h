// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TrafficIntersectionPoint.h"
#include "GameFramework/Actor.h"
#include "GroupedTrafficIntersectionPoint.generated.h"

class ATrafficIntersectionPoint;

UCLASS()
class WHEELMEALMANIA_API AGroupedTrafficIntersectionPoint : public ATrafficIntersectionPoint
{
	GENERATED_BODY()


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection")
	float HalfDistanceBetweenLanes = 600.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	UChildActorComponent* LeftLaneChildActorComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	UChildActorComponent* RightLaneChildActorComponent;

private:
	UPROPERTY()
	ATrafficIntersectionPoint* LeftLane;
	UPROPERTY()
	ATrafficIntersectionPoint* RightLane;
	
public:	
	AGroupedTrafficIntersectionPoint();

	UFUNCTION(BlueprintCallable, Category = "Intersection")
	TArray<ATrafficIntersectionPoint*> GetIntersectionPoints();
	
	bool AreIntersectionsValid() const;
	
	virtual void SetIntersectionController(AIntersectionController* NewIntersectionController) override;
	virtual void SetWaitingState(bool bNewIsWaiting) override;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

};