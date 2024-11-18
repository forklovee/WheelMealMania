// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/WheelComponent.h"

UWheelComponent::UWheelComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UWheelComponent::BeginPlay()
{
	Super::BeginPlay();

}


void UWheelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

