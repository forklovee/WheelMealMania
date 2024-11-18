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
	FrontLeftSuspensionSocket= CreateDefaultSubobject<USceneComponent>(FName("FrontLeftSuspensionSocket"));
	FrontLeftSuspensionSocket->SetupAttachment(FrontLeftWheelSocket);
	FrontLeftSuspensionSocket->SetRelativeLocation(FVector(0.f, 0.f, SpringLength));
	
	FrontRightWheelSocket = CreateDefaultSubobject<USceneComponent>(FName("FrontRightWheelSocket"));
	FrontRightWheelSocket->SetupAttachment(RootComponent);
	FrontRightWheelSocket->SetRelativeLocation(FVector(50.f, 20.f, 0.f));
	FrontRightSuspensionSocket = CreateDefaultSubobject<USceneComponent>(FName("FrontRightSuspensionSocket"));
	FrontRightSuspensionSocket->SetupAttachment(FrontRightWheelSocket);
	FrontRightSuspensionSocket->SetRelativeLocation(FVector(0.f, 0.f, SpringLength));

	BackLeftWheelSocket = CreateDefaultSubobject<USceneComponent>(FName("BackLeftWheelSocket"));
	BackLeftWheelSocket->SetupAttachment(RootComponent);
	BackLeftWheelSocket->SetRelativeLocation(FVector(-50.f, -20.f, 0.f));
	BackLeftSuspensionSocket = CreateDefaultSubobject<USceneComponent>(FName("BackLeftSuspensionSocket"));
	BackLeftSuspensionSocket->SetupAttachment(BackLeftWheelSocket);
	BackLeftSuspensionSocket->SetRelativeLocation(FVector(0.f, 0.f, SpringLength));

	BackRightWheelSocket = CreateDefaultSubobject<USceneComponent>(FName("BackRightWheelSocket"));
	BackRightWheelSocket->SetupAttachment(RootComponent);
	BackRightWheelSocket->SetRelativeLocation(FVector(-50.f, 20.f, 0.f));
	BackRightSuspensionSocket = CreateDefaultSubobject<USceneComponent>(FName("BackRightSuspensionSocket"));
	BackRightSuspensionSocket->SetupAttachment(BackRightWheelSocket);
	BackRightSuspensionSocket->SetRelativeLocation(FVector(0.f, 0.f, SpringLength));
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

	// Check if the vehicle is grounded.
	bool bNewIsOnGround = IsOnGround();
	if (bNewIsOnGround != bIsOnGround) {
		if (bNewIsOnGround) {
			JumpCounter = 0;
			OnLanded();
		}
		else {
			OnInAir();
		}
	}
	bIsOnGround = bNewIsOnGround;

	// Manually add gravity
	// General gravity
	VehicleCollision->AddForceAtLocation(
		FVector::DownVector * 981.f * 2250.f,
		VehicleCollision->GetComponentLocation()
	);
	// Hood gravity
	VehicleCollision->AddForceAtLocation(
		FVector::DownVector * 981.f * 250.f,
		VehicleCollision->GetComponentLocation() + VehicleCollision->GetForwardVector() * VehicleCollision->GetScaledBoxExtent().X
	);

	VehicleCollision->SetEnableGravity(false);

	// Process Steering
	Steering = FMath::Lerp(Steering, TargetSteering, DeltaTime*SteeringSensitivity);

	// MAIN PROCESSING
	SuspensionCast(DeltaTime);
	UpdateWheelsVelocityAndDirection(DeltaTime); // Updates TargetDriveForce
	// MAIN PROCESSING

	float TargetAcceleration = bIsOnGround ? Throttle : 0.f;
	float AccelerationRate = bIsThrottling ? ThrottleAccelerationRate : IdleEngineBreakingRate;
	AccelerationRate = IsAnyWheelOnTheGround() ? AccelerationRate : 0.01;
	
	Acceleration = FMath::FInterpTo(Acceleration, TargetAcceleration, DeltaTime, AccelerationRate);

	if (bIsOnGround) {
		MomentumDirection = (GetVelocity().GetSafeNormal() + FVector::DownVector*.25f).GetSafeNormal();
	}
}

// Called to bind functionality to input
void ABaseVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInput->BindAction(ThrottleInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::ThrottleInput);
		EnhancedInput->BindAction(ThrottleInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::ThrottleInput);

		EnhancedInput->BindAction(BreakInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::BreakInput);
		EnhancedInput->BindAction(BreakInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::BreakInput);

		EnhancedInput->BindAction(SteeringInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::SteeringInput);
		EnhancedInput->BindAction(SteeringInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::SteeringInput);

		EnhancedInput->BindAction(HandBreakInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::HandbreakInput);
		EnhancedInput->BindAction(HandBreakInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::HandbreakInput);
		
		EnhancedInput->BindAction(JumpInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::JumpingInput);
		
		EnhancedInput->BindAction(LookAroundInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::LookAroundInput);
		EnhancedInput->BindAction(LookAroundInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::LookAroundInput);

		EnhancedInput->BindAction(ResetCameraInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::ResetCameraInput);
	}
}

float ABaseVehicle::GetCurrentTargetSpeed()
{
	return Acceleration*MaxSpeed;
}

void ABaseVehicle::SuspensionCast(float DeltaTime)
{
	for (USceneComponent* WheelSocket : { FrontLeftWheelSocket, FrontRightWheelSocket, BackLeftWheelSocket, BackRightWheelSocket }) {
		if (USceneComponent* SuspensionSocket = WheelSocket->GetChildComponent(0)) {
			float SuspensionSocketDistance = -GetVelocity().Z * 0.05;
			SuspensionSocketDistance = FMath::Clamp(SuspensionSocketDistance, SpringLength*.5f, SpringLength - SocketDistanceOffset*.5f);
			SuspensionSocket->SetRelativeLocation(FVector(0.f, 0.f, -SuspensionSocketDistance));
		}

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

		if (USceneComponent* SuspensionSocket = WheelSocket->GetChildComponent(0)) {
			float SuspensionSocketDistance = (TraceHitResult.TraceStart - TraceHitResult.ImpactPoint).Length() - SocketDistanceOffset;
			SuspensionSocket->SetRelativeLocation(FVector(0.f, 0.f, -SuspensionSocketDistance));
		}
	}
}

void ABaseVehicle::UpdateWheelsVelocityAndDirection(float DeltaTime)
{
	float TargetSpeed = MaxSpeed * Acceleration * VehicleCollision->GetMass();
	
	uint8 iCollidingWheels = 0;
	for (USceneComponent* WheelSocket : { BackLeftWheelSocket, BackRightWheelSocket, FrontLeftWheelSocket, FrontRightWheelSocket }) {
		
		// Wheel Trace Test
		FHitResult TraceHitResult;
		bool bWheelOnGround = WheelCast(WheelSocket, TraceHitResult);

		// Add drive force, if wheel touches the ground
		FVector WheelForward = VehicleCollision->GetForwardVector();
		UKismetSystemLibrary::DrawDebugArrow(
			this,
			TraceHitResult.TraceStart,
			TraceHitResult.TraceStart + WheelForward * 200,
			120.f,
			FLinearColor::Red
		);

		if (!bWheelOnGround) {
			continue;
		}
		iCollidingWheels++;

		WheelForward = VehicleCollision->GetRightVector().Cross(TraceHitResult.ImpactNormal);

		FVector WheelForce = WheelForward * TargetSpeed * .25f;

		// Apply Drive Force
		VehicleCollision->AddForceAtLocation(
			WheelForce, WheelSocket->GetComponentLocation()
		);

		// Draw wheel force
		UKismetSystemLibrary::DrawDebugArrow(
			this,
			TraceHitResult.TraceStart,
			TraceHitResult.TraceStart + WheelForce,
			55.f,
			FLinearColor::Yellow
		);

		// Draw wheel direction
		UKismetSystemLibrary::DrawDebugArrow(
			this,
			TraceHitResult.TraceStart,
			TraceHitResult.TraceStart + WheelForward * 100.0f,
			15.f,
			FLinearColor::Green
		);
	}

	if (!bIsOnGround) {
		FVector MomentumForceOrigin = VehicleCollision->GetComponentLocation() +
			VehicleCollision->GetScaledBoxExtent().X * VehicleCollision->GetForwardVector();
		FVector MomentumForce = MomentumDirection * TargetSpeed;
		VehicleCollision->AddForceAtLocation(MomentumForce, MomentumForceOrigin);

		UKismetSystemLibrary::DrawDebugArrow(
			this,
			MomentumForceOrigin,
			MomentumForceOrigin + MomentumForce,
			250.f,
			FLinearColor::Blue
		);
	}
	
	// Process steering torque
	for (USceneComponent* WheelSocket : { FrontLeftWheelSocket, FrontRightWheelSocket }) {
		FHitResult TraceHitResult;
		if (!WheelCast(WheelSocket, TraceHitResult)) {
			continue;
		}

		// Turn Vehicle
		VehicleCollision->AddTorqueInRadians(
			FVector(
				0.f,
				0.f,
				Steering.X * 2.5f * 100000000.f
			) * Acceleration
		);
	}

	FVector VehicleMassCenter = MassCenterOffset + FVector(50.f, 0.f, 0.f);
	//FVector VehicleMassCenter = MassCenterOffset - FVector(VehicleCollision->GetScaledBoxExtent().X*1.f, 0.f, 0.f) * Acceleration;

	UKismetSystemLibrary::DrawDebugArrow(this,
		VehicleCollision->GetComponentLocation() + VehicleMassCenter.ProjectOnTo(VehicleCollision->GetForwardVector()),
		VehicleCollision->GetComponentLocation() + VehicleMassCenter.ProjectOnTo(VehicleCollision->GetForwardVector()) + FVector::UpVector * 150.f,
		128.f, FLinearColor::Blue);

	// Update Mass Center
	if (IsAnyWheelOnTheGround()) {
		VehicleCollision->SetCenterOfMass(VehicleMassCenter);
	}
}

float ABaseVehicle::GetTargetWheelSpeed()
{
	return Acceleration * MaxSpeed * VehicleCollision->GetMass();
}

FVector ABaseVehicle::GetTargetVelocity()
{
	return VehicleCollision->GetForwardVector() * GetTargetWheelSpeed();
}

FVector ABaseVehicle::GetHorizontalVelocity()
{
	return GetVelocity() * FVector(1.f, 1.f, 0.f);
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

bool ABaseVehicle::IsOnGround()
{
	uint8 WheelsOnGround = 0;
	for (USceneComponent* WheelSocket : { FrontLeftWheelSocket, FrontRightWheelSocket, BackLeftWheelSocket, BackRightWheelSocket }) {
		FHitResult HitResult;
		if (WheelCast(WheelSocket, HitResult)) {
			WheelsOnGround++;
		}
	}
	return WheelsOnGround == 4;
}

bool ABaseVehicle::IsAnyWheelOnTheGround()
{
	for (USceneComponent* WheelSocket : { FrontLeftWheelSocket, FrontRightWheelSocket, BackLeftWheelSocket, BackRightWheelSocket }) {
		FHitResult HitResult;
		if (WheelCast(WheelSocket, HitResult)) {
			return true;
		}
	}
	return false;
}

void ABaseVehicle::SteeringInput(const FInputActionValue& InputValue)
{
	TargetSteering = InputValue.Get<FVector2D>();

	OnSteeringUpdate(Steering);
}

void ABaseVehicle::ThrottleInput(const FInputActionValue& InputValue)
{
	Throttle = InputValue.Get<float>();
	bIsThrottling = Throttle > 0.0;

	OnThrottleUpdate(Throttle);
}

void ABaseVehicle::BreakInput(const FInputActionValue& InputValue)
{
	bIsBreaking = InputValue.Get<float>() > 0.0;
	if (bIsBreaking){
		Throttle = 0.f;
	}
	else {
		Throttle = FMath::Clamp(Throttle, 0.0, 1.0);
	}

	OnBreaking();
}

void ABaseVehicle::HandbreakInput(const FInputActionValue& InputValue)
{
	bIsHandbreaking = InputValue.Get<bool>();

	if (bIsHandbreaking) {
		Throttle = FMath::Clamp(Throttle, 0.0, .05f);
	}
	else {
		Throttle = FMath::Clamp(Throttle, 0.0, 1.0);
	}

	OnHandbreaking();
}

void ABaseVehicle::JumpingInput(const FInputActionValue& InputValue)
{
	if (!bIsOnGround || JumpCounter > 0) {
		return;
	}

	JumpCounter++;

	float TargetJumpStrength = VehicleCollision->GetMass() * JumpStrength;
	FVector JumpForce = VehicleCollision->GetUpVector() * TargetJumpStrength;
	VehicleCollision->SetCenterOfMass(MassCenterOffset + FVector(50.f, 0.f, 0.f));
	VehicleCollision->AddForceAtLocation(
		JumpForce * 2.f * GetWorld()->GetDeltaSeconds() * 4000.f,
		VehicleCollision->GetComponentLocation()
	);

	OnJumped();
}

void ABaseVehicle::SideBalanceInput(const FInputActionValue& InputValue)
{
	TargetSideBalance = InputValue.Get<float>();

	OnSideBalanceChanged(TargetSideBalance);
}

void ABaseVehicle::LookAroundInput(const FInputActionValue& InputValue)
{
	FVector2D LookAroundVector = InputValue.Get<FVector2D>() * 10.f;

	CameraArm->AddLocalRotation(FRotator(0.f, LookAroundVector.X, 0.f));
}

void ABaseVehicle::ResetCameraInput(const FInputActionValue& InputValue)
{
	CameraArm->SetRelativeRotation(FRotator::ZeroRotator);
}
