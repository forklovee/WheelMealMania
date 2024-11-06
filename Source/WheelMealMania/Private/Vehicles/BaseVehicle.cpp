// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"
#include "InputAction.h"

#include "Vehicles/BaseVehicle.h"
#include <EnhancedInputComponent.h>

// Sets default values
ABaseVehicle::ABaseVehicle()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VehicleCollision = CreateDefaultSubobject<UBoxComponent>(FName("CollisionBox"));
	VehicleCollision->SetLinearDamping(2.f);
	RootComponent = VehicleCollision;
	
	VehicleMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("VehicleMesh"));
	VehicleMesh->SetupAttachment(RootComponent);
	
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(FName("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	Camera->SetupAttachment(CameraArm);

	//Setup Suspension Sockets
	FrontLeftWheelSocket = CreateDefaultSubobject<USceneComponent>(FName("FrontLeftWheelSocket"));
	FrontLeftWheelSocket->SetupAttachment(RootComponent);
	FrontLeftWheelSocket->SetRelativeLocation(FVector(50.f, -20.f, 0.f));
	FrontRightWheelSocket = CreateDefaultSubobject<USceneComponent>(FName("FrontRightWheelSocket"));
	FrontRightWheelSocket->SetupAttachment(RootComponent);
	FrontRightWheelSocket->SetRelativeLocation(FVector(50.f, 20.f, 0.f));
	BackLeftWheelSocket = CreateDefaultSubobject<USceneComponent>(FName("BackLeftWheelSocket"));
	BackLeftWheelSocket->SetupAttachment(RootComponent);
	BackLeftWheelSocket->SetRelativeLocation(FVector(-50.f, -20.f, 0.f));
	BackRightWheelSocket = CreateDefaultSubobject<USceneComponent>(FName("BackRightWheelSocket"));
	BackRightWheelSocket->SetupAttachment(RootComponent);
	BackRightWheelSocket->SetRelativeLocation(FVector(-50.f, 20.f, 0.f));

}

// Called when the game starts or when spawned
void ABaseVehicle::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Acceleration = FMath::FInterpTo(Acceleration, Throttle, DeltaTime,
		bIsThrottling ? ThrottleAccelerationRate : IdleEngineBreakingRate);
	Steering = FMath::FInterpTo(Steering, TargetSteering, DeltaTime, SteeringSensitivity);

	SuspensionCast();
	UpdateWheelsVelocityAndDirection();
}

// Called to bind functionality to input
void ABaseVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInput->BindAction(SteeringInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::SteeringInput);
		EnhancedInput->BindAction(SteeringInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::SteeringInput);

		EnhancedInput->BindAction(ThrottleInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::ThrottleInput);
		EnhancedInput->BindAction(ThrottleInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::ThrottleInput);
	}
}

void ABaseVehicle::SuspensionCast()
{
	for (USceneComponent* WheelSocket : { FrontLeftWheelSocket, FrontRightWheelSocket, BackLeftWheelSocket, BackRightWheelSocket }) {
		FHitResult TraceHitResult;
		if (!WheelCast(WheelSocket, TraceHitResult)) {
			continue;
		}
		
		float DistanceNormalized = FMath::GetMappedRangeValueClamped(FVector2D(0.0, SpringLength), FVector2D(0.0, 1.0), TraceHitResult.Distance);
		float DistanceInversed = 1.f - DistanceNormalized;

		FVector TraceOffset = TraceHitResult.TraceStart - TraceHitResult.TraceEnd;
		FVector TraceDirection = TraceHitResult.Normal;
		FVector TargetForce = DistanceInversed * VehicleCollision->GetMass() * SpringStrength * TraceDirection;
		VehicleCollision->AddForceAtLocation(
			TargetForce,
			WheelSocket->GetComponentLocation()
		);
	}
}

void ABaseVehicle::UpdateWheelsVelocityAndDirection()
{
	float TargetSpeed = Acceleration * MaxSpeed * VehicleCollision->GetMass();
	for (USceneComponent* WheelSocket : { BackLeftWheelSocket, BackRightWheelSocket }) {
		FHitResult TraceHitResult;
		if (!WheelCast(WheelSocket, TraceHitResult)) {
			continue;
		}

		FVector Forward = TraceHitResult.ImpactNormal.Cross(FVector::RightVector);
		VehicleCollision->AddForceAtLocation(
			Forward * TargetSpeed,
			TraceHitResult.TraceStart
		);
	}

	for (USceneComponent* WheelSocket : { FrontLeftWheelSocket, FrontRightWheelSocket }) {
		FHitResult TraceHitResult;
		if (!WheelCast(WheelSocket, TraceHitResult)) {
			continue;
		}

		FVector Forward = TraceHitResult.ImpactNormal.Cross(FVector::RightVector);
		Forward.RotateAngleAxis(WheelMaxAngleDeg * Steering, FVector::UpVector);
		VehicleCollision->AddForceAtLocation(
			Forward * TargetSpeed,
			TraceHitResult.TraceStart
		);
	}

	// Update Mass Center
	VehicleCollision->SetCenterOfMass(
		MassCenterOffset - FVector::ForwardVector * .5f * VehicleCollision->GetScaledBoxExtent().X * Acceleration
	);
}

bool ABaseVehicle::WheelCast(USceneComponent* WheelSocket, FHitResult& HitResult)
{
	UKismetSystemLibrary::SphereTraceSingle(
		this,
		WheelSocket->GetComponentLocation(),
		WheelSocket->GetComponentLocation() - (SpringLength*VehicleCollision->GetUpVector()),
		WheelRadius,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		{ this },
		EDrawDebugTrace::ForOneFrame,
		HitResult,
		true,
		FLinearColor::Blue,
		FLinearColor::Red,
		0.1f
	);
	return HitResult.bBlockingHit;
}

void ABaseVehicle::SteeringInput(const FInputActionValue& InputValue)
{
	TargetSteering = InputValue.Get<float>();

	OnSteeringUpdate(Steering);
}

void ABaseVehicle::ThrottleInput(const FInputActionValue& InputValue)
{
	Throttle = InputValue.Get<float>();
	bIsThrottling = FMath::Abs(Throttle) > 0.0;

	OnThrottleUpdate(Throttle);
}

void ABaseVehicle::JumpingInput(const FInputActionValue& InputValue)
{
	OnJumped();
}
