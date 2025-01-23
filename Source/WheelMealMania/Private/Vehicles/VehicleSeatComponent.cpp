// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/VehicleSeatComponent.h"

#include "MovieSceneTracksComponentTypes.h"

// Sets default values for this component's properties
UVehicleSeatComponent::UVehicleSeatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UVehicleSeatComponent::Sit(AActor* ActorToSit)
{
	OwningActor = ActorToSit;

	OwningActor->SetActorEnableCollision(false);
	OwningActor->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void UVehicleSeatComponent::RemoveActorFromSeat()
{
	AActor* Actor = GetSittingActor();
	if (!Actor)
	{
		return;
	}
	Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	OwningActor = nullptr;
}

AActor* UVehicleSeatComponent::GetSittingActor() const
{
	return OwningActor.Get();
}


// Called when the game starts
void UVehicleSeatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


