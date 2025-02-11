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

void UWheelComponent::Setup(UBoxComponent* NewVehicleCollision, float NewMaxWheelRotationAngleDeg,
	float NewGravityScale,
	bool bNewDrawDebug)
{
	VehicleCollision = NewVehicleCollision;
	MaxWheelRotationAngleDeg = NewMaxWheelRotationAngleDeg;
	GravityScale = NewGravityScale;
	bDrawDebug = bNewDrawDebug;
}

void UWheelComponent::Update(const float& NewSpeed, const float& NewAngle)
{
	TargetSpeed = NewSpeed;
	Angle = NewAngle;
}

float UWheelComponent::GetGravityForce() const
{
	return 981.f * 1050.0 * GravityScale;
}

void UWheelComponent::SetDriftMode(bool bNewDrifting)
{
	bIsDrifting = bNewDrifting;

	// FrictionOverride = (bIsDrifting) ? 0.f : 1.f;
}

void UWheelComponent::SetSpringPointingDown(bool bNewSpringPointingDown)
{
	bSpringPointingDown = bNewSpringPointingDown;
}

void UWheelComponent::BeginPlay()
{
	Super::BeginPlay();
	
	SetComponentTickEnabled(true);
	TargetWheelHeight = GetSpringLength();
}

void UWheelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CurrentFriction = FMath::FInterpTo(
		CurrentFriction,
		FrictionCoefficient*GroundFriction*FrictionOverride,
		DeltaTime, 10.f);

	UpdateWheelCollisionCast();
	UpdateWheelGravity();
	UpdateWheelForwardForce(DeltaTime);

	CurrentWheelHeight = FMath::FInterpTo(CurrentWheelHeight, TargetWheelHeight, DeltaTime, 5.f);
	
	if (bAffectedBySteering)
	{
		SetRelativeRotation( FRotator(0.f, Angle, 0.f) );
	}
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

void UWheelComponent::UpdateWheelForwardForce(float DeltaTime)
{
	const float VehicleMass = VehicleCollision->GetMass();
	if (!IsOnGround())
	{
		VehicleCollision->AddForceAtLocation(
			VehicleCollision->GetComponentVelocity() * VehicleMass * 1.1f,
			GetComponentLocation());
		return;
	}

	const FVector WheelForward = GetForwardVector();
	const FVector WheelForwardForce = WheelForward * TargetSpeed * VehicleCollision->GetMass() * DeltaTime;
	VehicleCollision->AddForceAtLocation(
		WheelForwardForce,
		GetComponentLocation());

	const FVector WheelRight = GetRightVector();
	const float SideDragAcceleration = WheelRight.Dot(VehicleCollision->GetComponentVelocity() * VehicleMass);
	const float SideSlideDirection = (bIsDrifting && !bAffectedBySteering) ? -1.f : 1.f;
	const FVector SideDragForce = SideSlideDirection * CurrentFriction * SideDragAcceleration * -WheelRight;
	VehicleCollision->AddForceAtLocation(
		SideDragForce,
		GetComponentLocation());
	
	if (bDrawDebug)
	{
		UKismetSystemLibrary::DrawDebugArrow(
		this,
		GetComponentLocation() + FVector::UpVector * 100.f,
		GetComponentLocation() + FVector::UpVector * 100.f + WheelForwardForce,
		55.f,
		FLinearColor::Blue);
	
		UKismetSystemLibrary::DrawDebugArrow(
		this,
		GetComponentLocation() + FVector::UpVector * 100.f,
		GetComponentLocation() + FVector::UpVector * 100.f + SideDragForce,
		55.f,
		FLinearColor::Red);
	}
	
}

void UWheelComponent::UpdateSteering(float DeltaTime)
{
	
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

void UWheelComponent::SetSpringStrengthRatio(float NewSpringStrengthRatio)
{
	SpringStrengthRatio = NewSpringStrengthRatio;
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

	if (!VehicleCollision.IsValid()) {
		return false;
	}

	GroundNormal = FMath::Lerp(GroundNormal, WheelHitResult.ImpactNormal, 0.5f);
	TargetWheelHeight = WheelHitResult.Distance;
	float DistanceNormalized = FMath::GetMappedRangeValueClamped(FVector2D(0.0, SpringLength), FVector2D(0.0, 1.0), TargetWheelHeight);
	float DistanceInversed = 1.f - DistanceNormalized;

	const FVector TraceOffset = WheelHitResult.TraceStart - WheelHitResult.TraceEnd;
	const FVector TraceDirection = TraceOffset.GetSafeNormal();
	const FVector TargetForce = DistanceInversed * VehicleCollision->GetMass() * FMath::Lerp(0.f, SpringStrength, SpringStrengthRatio) * TraceDirection;
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
	const FVector SpringDownVector = (!bSpringPointingDown) ? -GetUpVector() : FVector::DownVector;
	UKismetSystemLibrary::SphereTraceSingle(
		this,
		GetComponentLocation(),
		GetComponentLocation() + (FMath::Lerp(0.f, SpringLength, SpringLengthRatio) * SpringDownVector),
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

