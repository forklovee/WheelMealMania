// Fill out your copyright notice in the Description page of Project Settings.

#include "Vehicles/WheelComponent.h"

#include "Kismet/KismetSystemLibrary.h"
#include <Components/BoxComponent.h>


UWheelComponent::UWheelComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	if (!ComponentHasTag(FName("Wheel"))) {
		ComponentTags.Add(FName("Wheel"));
	}
}


void UWheelComponent::BeginPlay()
{
	Super::BeginPlay();

	VehicleCollision = Cast<UBoxComponent>(GetAttachParent());
	if (!VehicleCollision) {
		UE_LOG(LogTemp, Error, TEXT("WheelComponent: Can't find parent Vehicle Collision."));
		SetComponentTickEnabled(false);
		return;
	}

	SetComponentTickEnabled(true);
	TargetWheelHeight = GetSpringLength();
}

void UWheelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateWheelCollisionCast();

	CurrentWheelHeight = FMath::FInterpTo(CurrentWheelHeight, TargetWheelHeight, DeltaTime, 5.f);
}

float UWheelComponent::GetSpringLengthRatio()
{
	return SpringLengthRatio;
}

float UWheelComponent::GetSpringLength()
{
	return FMath::Lerp(0.f, SpringLength, SpringLengthRatio);
}

void UWheelComponent::SetSpringLengthRatio(float NewSpringLengthRatio)
{
	SpringLengthRatio = NewSpringLengthRatio;

	OnSpringLengthUpdated(GetSpringLength());
	OnSpringLengthUpdatedDelegate.Broadcast(GetSpringLength());
}

bool UWheelComponent::IsOnGround()
{
	return bIsOnGround;
}

FVector UWheelComponent::GetGroundNormalVector()
{
	return GroundNormal;
}

bool UWheelComponent::UpdateWheelCollisionCast()
{
	FHitResult WheelHitResult;
	UKismetSystemLibrary::SphereTraceSingle(
		this,
		GetComponentLocation(),
		GetComponentLocation() - (SpringLength * GetUpVector()),
		WheelRadius,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		{ GetOwner() },
		EDrawDebugTrace::ForOneFrame,
		WheelHitResult,
		true,
		FLinearColor::Blue,
		FLinearColor::Red,
		0.1f
	);
	
	bool bLastIsOnGround = bIsOnGround;
	bIsOnGround = WheelHitResult.bBlockingHit;

	if (!bIsOnGround) {

		//Was on ground, is in air.
		if (bLastIsOnGround) {
			OnWheelInAirDelegate.Broadcast();
			OnWheelInAir();
		}

		TargetWheelHeight = GetSpringLength();
		return false;
	}

	//Was in air, is on ground.
	if (!bLastIsOnGround) {
		OnWheelOnGroundDelegate.Broadcast();
		OnWheelOnGround();
	}

	if (!VehicleCollision) {
		return false;
	}

	GroundNormal = WheelHitResult.ImpactNormal;
	TargetWheelHeight = WheelHitResult.Distance;

	float DistanceNormalized = FMath::GetMappedRangeValueClamped(FVector2D(0.0, SpringLength), FVector2D(0.0, 1.0), TargetWheelHeight);
	float DistanceInversed = 1.f - DistanceNormalized;

	FVector TraceOffset = WheelHitResult.TraceStart - WheelHitResult.TraceEnd;
	FVector TraceDirection = WheelHitResult.Normal;
	FVector TargetForce = DistanceInversed * VehicleCollision->GetMass() * SpringStrength * TraceDirection;
	VehicleCollision->AddForceAtLocation(
		TargetForce,
		GetComponentLocation()
	);

	return true;
}

