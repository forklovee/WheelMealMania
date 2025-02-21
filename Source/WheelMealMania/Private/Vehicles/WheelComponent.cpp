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

float UWheelComponent::GetPhysicsForceDeltaTimeScaler() const
{
	return (1.f/24.f) / (GetWorld()->GetDeltaSeconds());
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

void UWheelComponent::Jump(float JumpForce)
{
	if (!bIsOnGround)
	{
		return;
	}
	const FVector JumpForceVector = 10.f*JumpForce * VehicleCollision->GetMass() * GetUpVector();
	VehicleCollision->AddForceAtLocation(JumpForceVector, GetComponentLocation());
	SetSpringStrengthRatio(1.f);
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

	if (!VehicleCollision.IsValid())
	{
		Setup(Cast<UBoxComponent>(GetAttachParent()), 0.f, 1.f);
	}
	if (!VehicleCollision.IsValid())
	{
		return;
	}
	
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
	if (GetComponentVelocity().Length() < 100.f && WheelHitResult.bBlockingHit && SurfaceDot > MaxSteepness)
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
	if (!bIsOnGround)
	{
		VehicleCollision->AddForceAtLocation(
			VehicleCollision->GetComponentVelocity() * VehicleMass * 2.25f,
			GetComponentLocation());
		return;
	}
	
	//Ramp force scaling
	const float GroundDot = FVector::UpVector.Dot(GetUpVector());
	const float MaxSteepness = 0.1f;
	float ForceGroundScaler = FMath::GetMappedRangeValueClamped(
		FVector2D(MaxSteepness, 1.0),
		FVector2D(0.f, 1.f),
		GroundDot);
	
	const FVector WheelForward = GetForwardVector();
	const FVector WheelForwardForce = 10.f * ForceGroundScaler * WheelForward * TargetSpeed;
	VehicleCollision->AddForceAtLocation(
		WheelForwardForce,
		GetComponentLocation());

	const FVector WheelRight = GetRightVector();
	const float SideDragAcceleration = WheelRight.Dot(VehicleCollision->GetComponentVelocity());
	const float SideSlideDirection = (bIsDrifting && !bAffectedBySteering) ? -0.35f : 1.f;
	const FVector SideDragForce = 10000.f * SideSlideDirection * 1.f * SideDragAcceleration * -WheelRight;
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

	GroundNormal = WheelHitResult.Normal;
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
	UKismetSystemLibrary::LineTraceSingle(
		this,
		GetComponentLocation(),
		GetComponentLocation() + (FMath::Lerp(0.f, SpringLength, SpringLengthRatio) * SpringDownVector),
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

