// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/TrafficIntersectionPoint.h"
#include "Vehicles/IntersectionController.h"

bool ATrafficIntersectionPoint::IsDisabled() const
{
	return bDisabled;
}

AIntersectionController* ATrafficIntersectionPoint::GetIntersectionController() const
{
	return IntersectionController.Get();
}

void ATrafficIntersectionPoint::SetIntersectionController(AIntersectionController* NewIntersectionController)
{
	IntersectionController = NewIntersectionController;

	UE_LOG(LogTemp, Display, TEXT("%s IntersectionController Set!"), *GetName());
}

bool ATrafficIntersectionPoint::GetWaitingState() const
{
	return bIsWaiting;
}

void ATrafficIntersectionPoint::SetWaitingState(bool bNewIsWaiting)
{
	bIsWaiting = bNewIsWaiting;
	
	OnWaitingStateChanged(bNewIsWaiting);
	WaitStateChanged.Broadcast(this, bNewIsWaiting);
}

void ATrafficIntersectionPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (bIsRightLane)
	{
		if (!ActorHasTag("RightLane")){
			Tags.Add(FName("RightLane"));
		}
		if (ActorHasTag("LeftLane"))
		{
			Tags.Remove(FName("LeftLane"));
		}
	}
	else
	{
		if (!ActorHasTag("LeftLane")){
			Tags.Add(FName("LeftLane"));
		}
		if (ActorHasTag("RightLane"))
		{
			Tags.Remove(FName("RightLane"));
		}
	}
}
