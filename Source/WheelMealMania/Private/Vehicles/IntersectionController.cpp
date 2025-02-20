// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/IntersectionController.h"

#include "Vehicles/GroupedTrafficIntersectionPoint.h"
#include "Vehicles/TrafficIntersectionPoint.h"

AIntersectionController::AIntersectionController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AIntersectionController::Start()
{
	SequenceIndex = -1;
	if (Sequences.IsEmpty())
	{
		return;
	}
	
	InitializeIntersectionPoints();
	StartNewSequence();
}

uint8 AIntersectionController::GetSequenceIndex() const
{
	return SequenceIndex;
}

TArray<ATrafficIntersectionPoint*> AIntersectionController::GetOpenIntersectionPointsOfLane(bool bRightLane)
{
	TArray<ATrafficIntersectionPoint*> OpenIntersectionPoints;
	for (ATrafficIntersectionPoint* IntersectionPoint : IntersectionPoints)
	{
		if (IntersectionPoint->IsDisabled() || IntersectionPoint->GetWaitingState()) continue;
		
		if ((bRightLane && IntersectionPoint->ActorHasTag("RightLane")) ||
			(!bRightLane && IntersectionPoint->ActorHasTag("LeftLane")))
		{
			UE_LOG(LogTemp, Display, TEXT("Right=%i Selected Open: %s"), bRightLane, *IntersectionPoint->GetName())
			OpenIntersectionPoints.Add(IntersectionPoint);
		}
	}
	
	return OpenIntersectionPoints;
}

void AIntersectionController::BeginPlay()
{
	Super::BeginPlay();
	
	if (bAutoStart)
	{
		GetWorldTimerManager().SetTimer(SequenceTimer, this, &AIntersectionController::Start, 1.f, false);
	}
}

void AIntersectionController::StartNewSequence()
{
	SequenceIndex++;
	
	GetWorldTimerManager().ClearTimer(SequenceTimer);
	GetWorldTimerManager().ClearTimer(TickTimer);
	
	if (SequenceIndex >= Sequences.Num())
	{
		Stop();
		return;
	}

	FIntersectionSequence* CurrentSequence = &Sequences[SequenceIndex];
	for (FIntersectionPointState& State : CurrentSequence->States)
	{
		ATrafficIntersectionPoint* IntersectionPoint = State.IntersectionPoint.Get();
		if (!IntersectionPoint)
		{
			continue;
		}
		if (!IntersectionPoints.Contains(IntersectionPoint))
		{
			IntersectionPoints.Add(IntersectionPoint);
		}
		
		IntersectionPoint->SetWaitingState(State.bIsWaiting);
	}

	GetWorldTimerManager().SetTimer(SequenceTimer, this, &AIntersectionController::StartNewSequence, static_cast<float>(CurrentSequence->Duration), false);
	GetWorldTimerManager().SetTimer(TickTimer, this, &AIntersectionController::SequenceTick, 1.f, true);
	
	SequenceStarted.Broadcast(this, *CurrentSequence);
}

void AIntersectionController::SequenceTick()
{
	const int SequenceDuration = Sequences[SequenceIndex].Duration;
	const int TimeLeft = SequenceDuration - static_cast<int>(GetWorldTimerManager().GetTimerElapsed(SequenceTimer));
	
	SequenceTimeTick.Broadcast(this, TimeLeft);
	OnSequenceTimeTick(Sequences[SequenceIndex], TimeLeft);
}

void AIntersectionController::Stop()
{
	GetWorldTimerManager().ClearTimer(SequenceTimer);
	GetWorldTimerManager().ClearTimer(TickTimer);

	SequenceIndex = -1;

	SequencesEnded.Broadcast(this);
	
	if (bLoop)
	{
		StartNewSequence();
	}
}

void AIntersectionController::InitializeIntersectionPoints()
{
	for (FIntersectionSequence& Sequence : Sequences)
	{
		for (FIntersectionPointState PointState: Sequence.States)
		{
			ATrafficIntersectionPoint* IntersectionPoint = PointState.IntersectionPoint.Get();
			if (!IntersectionPoint)
			{
				continue;
			}
			AGroupedTrafficIntersectionPoint* GroupedTrafficIntersectionPoint = Cast<AGroupedTrafficIntersectionPoint>(IntersectionPoint);
			if (GroupedTrafficIntersectionPoint != nullptr)
			{
				for (ATrafficIntersectionPoint* SubTrafficIntersectionPoint: GroupedTrafficIntersectionPoint->GetIntersectionPoints())
				{
					if (IntersectionPoints.Contains(SubTrafficIntersectionPoint))
					{
						continue;
					}
					IntersectionPoints.Add(SubTrafficIntersectionPoint);
					SubTrafficIntersectionPoint->SetIntersectionController(this);
				}
				continue;
			}
			
			if (IntersectionPoints.Contains(IntersectionPoint))
			{
				continue;
			}
			IntersectionPoints.Add(IntersectionPoint);
			IntersectionPoint->SetIntersectionController(this);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("%s Intersection Points initialized: %i"), *GetName(), IntersectionPoints.Num());
}

