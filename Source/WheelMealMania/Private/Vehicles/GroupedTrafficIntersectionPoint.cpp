// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/GroupedTrafficIntersectionPoint.h"
#include "Vehicles/TrafficIntersectionPoint.h"

AGroupedTrafficIntersectionPoint::AGroupedTrafficIntersectionPoint()
{
	LeftLaneChildActorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("LeftLaneChildActor"));
	LeftLaneChildActorComponent->SetupAttachment(DebugMesh);
	LeftLaneChildActorComponent->SetRelativeLocation(FVector(0.f, -HalfDistanceBetweenLanes, 0.f));
	
	RightLaneChildActorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("RightLaneChildActor"));
	RightLaneChildActorComponent->SetupAttachment(DebugMesh);
	LeftLaneChildActorComponent->SetRelativeLocation(FVector(0.f, HalfDistanceBetweenLanes, 0.f));
	
	PrimaryActorTick.bCanEverTick = false;
}

TArray<ATrafficIntersectionPoint*> AGroupedTrafficIntersectionPoint::GetIntersectionPoints()
{
	return {LeftLane, RightLane};
}

bool AGroupedTrafficIntersectionPoint::AreIntersectionsValid() const
{
	return LeftLane && RightLane;
}

void AGroupedTrafficIntersectionPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	LeftLaneChildActorComponent->SetRelativeLocation(FVector(0.f, -HalfDistanceBetweenLanes, 0.f));
	RightLaneChildActorComponent->SetRelativeLocation(FVector(0.f, HalfDistanceBetweenLanes, 0.f));
}

void AGroupedTrafficIntersectionPoint::BeginPlay()
{
	Super::BeginPlay();

	if (LeftLaneChildActorComponent->GetChildActor())
	{
		LeftLane = Cast<ATrafficIntersectionPoint>(LeftLaneChildActorComponent->GetChildActor());
	}
	if (RightLaneChildActorComponent->GetChildActor())
	{
		RightLane = Cast<ATrafficIntersectionPoint>(RightLaneChildActorComponent->GetChildActor());
	}
	
}

void AGroupedTrafficIntersectionPoint::SetIntersectionController(AIntersectionController* NewIntersectionController)
{
	Super::SetIntersectionController(NewIntersectionController);
	
	if (!AreIntersectionsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("%s Intersection casting went wrong :("), *GetName());
		return;
	}
	
	LeftLane->SetIntersectionController(NewIntersectionController);
	RightLane->SetIntersectionController(NewIntersectionController);
}

void AGroupedTrafficIntersectionPoint::SetWaitingState(bool bNewIsWaiting)
{
	Super::SetWaitingState(bNewIsWaiting);

	if (!AreIntersectionsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("%s Intersection casting went wrong :("), *GetName());
		return;
	}
	
	LeftLane->SetWaitingState(bNewIsWaiting);
	RightLane->SetWaitingState(bNewIsWaiting);
}
