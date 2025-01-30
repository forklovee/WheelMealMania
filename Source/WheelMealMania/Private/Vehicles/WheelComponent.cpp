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

void UWheelComponent::SetGravityScale(float NewGravityScale)
{
	GravityScale = NewGravityScale;
}

void UWheelComponent::SetTargetSpeed(float NewSpeed)
{
	TargetSpeed = NewSpeed;
}

void UWheelComponent::SetDrawDebug(bool bNewDrawDebug)
{
	bDrawDebug = bNewDrawDebug;
}

float UWheelComponent::GetGravityForce() const
{
	return 981.f * 1050.0 * GravityScale;
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
	UpdateWheelGravity();
	UpdateWheelForwardForce();
	
	CurrentWheelHeight = FMath::FInterpTo(CurrentWheelHeight, TargetWheelHeight, DeltaTime, 5.f);
}

void UWheelComponent::UpdateWheelGravity()
{
	const FHitResult WheelHitResult = GetWheelHitResult();
	
	const float MaxSteepness = 0.8f;
	const float SurfaceDot = FMath::Clamp(FVector::UpVector.Dot(GetUpVector()), 0.f, 1.f);

	FVector GravityDirection = FVector::DownVector;
	if (WheelHitResult.bBlockingHit && SurfaceDot > MaxSteepness)
	{
		GravityDirection = -WheelHitResult.Normal;
	}

	// Wheel gravity
	GravityForce = GravityDirection * GetGravityForce();
	VehicleCollision->AddForceAtLocation(GravityForce, GetComponentLocation());

	if (bDrawDebug)
	{
		UKismetSystemLibrary::DrawDebugArrow(
			this,
			GetComponentLocation(),
			GetComponentLocation() - GravityForce*0.001f,
			55.f,
			FLinearColor::Red
		);
	}
}

void UWheelComponent::UpdateWheelForwardForce()
{
	if (!bAffectedByEngine)
	{
		return;
	}
	// Add drive force, if wheel touches the ground
	FVector WheelForward = GetForwardVector();
	if (!bIsOnGround)
	{
		WheelForward.Z = 0.f;
		WheelForward = WheelForward.GetSafeNormal();
	}
	
	// Apply Drive Forces
	FVector WheelForce = WheelForward * TargetSpeed;
	VehicleCollision->AddForceAtLocation(
		WheelForce, GetComponentLocation()
	);

	// Draw wheel force
	if (bDrawDebug)
	{
		UKismetSystemLibrary::DrawDebugArrow(
			this,
			GetComponentLocation(),
			GetComponentLocation() + WheelForward * 100.f,
			55.f,
			FLinearColor::Yellow
		);
	}
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
	const FHitResult WheelHitResult = GetWheelHitResult();
	
	bool bLastIsOnGround = bIsOnGround;
	bIsOnGround = WheelHitResult.bBlockingHit;

	if (!bIsOnGround) {
		GroundNormal = FVector::UpVector;
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

	const FVector TraceOffset = WheelHitResult.TraceStart - WheelHitResult.TraceEnd;
	const FVector TraceDirection = TraceOffset.GetSafeNormal();
	const FVector TargetForce = DistanceInversed * VehicleCollision->GetMass() * SpringStrength * TraceDirection;
	VehicleCollision->AddForceAtLocation(
		TargetForce,
		GetComponentLocation()
	);
	
	if (bDrawDebug)
	{
		UKismetSystemLibrary::DrawDebugArrow(
			this,
			GetComponentLocation(),
			GetComponentLocation() + TargetForce * 0.01f,
			55.f,
			FLinearColor::Yellow
		);

		// UKismetSystemLibrary::DrawDebugArrow(
		// 	this,
		// 	GetComponentLocation(),
		// 	GetComponentLocation() + FrictionForce * 0.01f,
		// 	55.f,
		// 	FLinearColor::Yellow
		// );
	}
	
	return true;
}

FHitResult UWheelComponent::GetWheelHitResult() const
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
		(bDrawDebug) ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
		WheelHitResult,
		true,
		FLinearColor::Blue,
		FLinearColor::Red,
		0.1f
	);
	return WheelHitResult;
}

