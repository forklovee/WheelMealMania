// Fill out your copyright notice in the Description page of Project Settings.

#include "Vehicles/BaseVehicle.h"

#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "Vehicles/WheelComponent.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"
#include "InputAction.h"
#include "NiagaraComponent.h"

#include <EnhancedInputComponent.h>

// Sets default values
ABaseVehicle::ABaseVehicle()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VehicleCollision = CreateDefaultSubobject<UBoxComponent>(FName("CollisionBox"));
	VehicleCollision->SetLinearDamping(2.f);
	VehicleCollision->SetGenerateOverlapEvents(true);
	VehicleCollision->SetNotifyRigidBodyCollision(true);
	RootComponent = VehicleCollision;
	
	VehicleMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("VehicleMesh"));
	VehicleMesh->SetupAttachment(RootComponent);
	
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(FName("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	Camera->SetupAttachment(CameraArm);
}

// Called when the game starts or when spawned
void ABaseVehicle::BeginPlay()
{
	Super::BeginPlay();

	SetupVehicleWheelComponents();
	VehicleCollision->OnComponentHit.AddDynamic(this, &ABaseVehicle::VehicleHit);
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

	VehicleCollision->SetEnableGravity(false);

	// Process Steering
	Steering = FMath::Lerp(Steering, TargetSteering, DeltaTime*SteeringSensitivity);

	// MAIN PROCESSING
	UpdateAcceleration(DeltaTime);
	UpdateWheelsVelocityAndDirection(DeltaTime); // Updates TargetDriveForce
	// MAIN PROCESSING

	LastDrivingDirection = GetVelocity().GetSafeNormal();

	UKismetSystemLibrary::DrawDebugArrow(
		this,
		VehicleCollision->GetComponentLocation(),
		VehicleCollision->GetComponentLocation() + LastDrivingDirection * 300.f,
		120.f,
		FLinearColor::White
	);
}

// Called to bind functionality to input
void ABaseVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInput->BindAction(ThrottleInputAction, ETriggerEvent::Started, this, &ABaseVehicle::ThrottlePressedInput);
		EnhancedInput->BindAction(ThrottleInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::ThrottleInput);
		EnhancedInput->BindAction(ThrottleInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::ThrottleInput);

		EnhancedInput->BindAction(BreakInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::BreakInput);
		EnhancedInput->BindAction(BreakInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::BreakInput);

		EnhancedInput->BindAction(GearShiftInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::GearShiftInput);
		EnhancedInput->BindAction(GearShiftInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::GearShiftInput);

		EnhancedInput->BindAction(SteeringInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::SteeringInput);
		EnhancedInput->BindAction(SteeringInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::SteeringInput);

		EnhancedInput->BindAction(RotationControlInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::HydraulicsControlInput);
		EnhancedInput->BindAction(RotationControlInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::HydraulicsControlInput);

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
	return GetVelocity().Length();
}

EGearShift ABaseVehicle::GetCurrentGearShift()
{
	return CurrentShift;
}

void ABaseVehicle::OnConstruction(const FTransform& Transform)
{
	// In editor :^)
}

bool ABaseVehicle::IsThrottling()
{
	return bIsThrottling;
}

void ABaseVehicle::UpdateAcceleration(float DeltaTime)
{
	// Apply Throttle, when on ground
	float TargetAcceleration = bIsOnGround ? Throttle : 0.f;

	// Basic acceleration Forward throttle acceleration or idle engine.
	float NewAcceleration = bIsThrottling ? ThrottleAccelerationRate : IdleEngineBreakingRate;
	bool bVehicleHasStopped = GetHorizontalVelocity().Length() < 100.f;
	switch (GetCurrentGearShift())
	{
		case EGearShift::DRIVE:
			if (!bDrivingForwards && bVehicleHasStopped)
			{
				if (bVehicleHasStopped) {
					bDrivingForwards = true;
					UE_LOG(LogTemp, Warning, TEXT("Can go forward now."));
				}
				if (Acceleration > 0.f) {
					bDrivingForwards = true;
				}
			}
			TargetAcceleration = bDrivingForwards ? TargetAcceleration : 0.f;

			NewAcceleration = bDrivingForwards ? ThrottleAccelerationRate : IdleEngineBreakingRate;
			break;
		case EGearShift::REVERSE:
			if (bDrivingForwards && bVehicleHasStopped)
			{
				bDrivingForwards = false;
				UE_LOG(LogTemp, Warning, TEXT("Can reverse now."));
			}
			TargetAcceleration = !bDrivingForwards ? TargetAcceleration : 0.f;

			NewAcceleration = !bDrivingForwards ? ThrottleAccelerationRate : IdleEngineBreakingRate;
			break;
	}

	NewAcceleration = bIsOnGround ? NewAcceleration : InAirBreakingRate;

	Acceleration = FMath::FInterpTo(Acceleration, TargetAcceleration, DeltaTime, NewAcceleration);
}

void ABaseVehicle::UpdateWheelsVelocityAndDirection(float DeltaTime)
{
	float TurnAngleScale = 1.f;
	if (IsValid(SteeringRangeCurve)) {
		TurnAngleScale = SteeringRangeCurve->GetFloatValue(Acceleration);
	}

	FVector WheelMomentumSum = FVector::ZeroVector;
	FVector WheelForceSum = FVector::ZeroVector;

	float TargetSpeed = Acceleration * VehicleCollision->GetMass();
	TargetSpeed *= bDrivingForwards ? MaxSpeed : -MaxReverseSpeed;
	for (UWheelComponent* Wheel : Wheels) {
		
		bool bWheelOnGround = Wheel->IsOnGround();

		// Add drive force, if wheel touches the ground
		float WheelSpeed = TargetSpeed * .25f;
		FVector WheelForward = Wheel->GetForwardVector();

		float SteeringAngleScaler = 1.0f;
		if (FrontWheels.Contains(Wheel)) {
			SteeringAngleScaler = 1.f;
		}
		else {
			SteeringAngleScaler = Acceleration;

			if (bIsBreaking) {
				WheelSpeed *= 0.0f;
			}
		}

		float TurnAngle = WheelMaxAngleDeg * TurnAngleScale;
		if (BackWheels.Contains(Wheel)) {
			TurnAngle *= Acceleration*.05;
		}

		// WheelForward = WheelForward.RotateAngleAxis(1.f * TurnAngle * Steering.X, VehicleCollision->GetUpVector());

		FVector WheelGravityDirection = FVector::DownVector;
		FVector WheelGravity = WheelGravityDirection * 981.f * GravityScale;
		if (!bWheelOnGround) {
			WheelGravity *= 900.f;
			VehicleCollision->AddForceAtLocation(WheelGravity, Wheel->GetComponentLocation());

			WheelMomentumSum += (LastDrivingDirection * WheelSpeed) * 4.f;
			WheelForceSum += LastDrivingDirection * WheelSpeed;
			continue;
		}
		WheelGravityDirection = -Wheel->GetGroundNormalVector();
		WheelGravity *= 1050.f;

		VehicleCollision->AddForceAtLocation(WheelGravity, Wheel->GetComponentLocation());

		// Apply Drive Forces
		FVector WheelForce = WheelForward * TargetSpeed;

		VehicleCollision->AddForceAtLocation(
			WheelForce, Wheel->GetComponentLocation()
		);
		WheelForceSum += WheelForce;

		// Draw wheel force
		UKismetSystemLibrary::DrawDebugArrow(
			this,
			Wheel->GetComponentLocation(),
			Wheel->GetComponentLocation() + WheelForce * 0.001f,
			55.f,
			FLinearColor::Yellow
		);
	}

	// Torque Turn Vehicle
	float TorqueForce = SteeringTorqueForce * WheelMaxAngleDeg;
	VehicleCollision->AddTorqueInRadians(
		FVector(
			0.f,
			0.f,
			Steering.X * TorqueForce * 10000000.f
		)
	);

	// Add Momentum
	VehicleCollision->AddForceAtLocation(
		WheelMomentumSum, VehicleCollision->GetComponentLocation());

	UKismetSystemLibrary::DrawDebugArrow(
		this,
		VehicleCollision->GetComponentLocation(),
		VehicleCollision->GetComponentLocation() + WheelMomentumSum * 200.f,
		120.f,
		FLinearColor::Gray
	);

	// Desired Velocity
	UKismetSystemLibrary::DrawDebugArrow(
		this,
		VehicleCollision->GetComponentLocation(),
		VehicleCollision->GetComponentLocation() + WheelForceSum * 200.f,
		120.f,
		FLinearColor::Green
	);

	UKismetSystemLibrary::DrawDebugArrow(
		this,
		VehicleCollision->GetComponentLocation(),
		VehicleCollision->GetComponentLocation() + GetVelocity(),
		120.f,
		FLinearColor::Red
	);

	FVector VehicleMassCenter = MassCenterOffset;
	UKismetSystemLibrary::DrawDebugArrow(this,
		VehicleCollision->GetComponentLocation() + VehicleMassCenter.ProjectOnTo(VehicleCollision->GetForwardVector()),
		VehicleCollision->GetComponentLocation() + VehicleMassCenter.ProjectOnTo(VehicleCollision->GetForwardVector()) + FVector::UpVector * 150.f,
		128.f, FLinearColor::Blue);

	UKismetSystemLibrary::DrawDebugBox(
		this,
		VehicleCollision->GetComponentLocation(),
		FVector::OneVector * 50.f,
		FLinearColor::Blue,
		VehicleCollision->GetForwardVector().ToOrientationRotator(),
		0.f,
		25.f
	);

	UKismetSystemLibrary::DrawDebugBox(
		this,
		VehicleCollision->GetComponentLocation() + VehicleMassCenter.ProjectOnTo(VehicleCollision->GetForwardVector()),
		FVector::OneVector * 50.f,
		FLinearColor::Red,
		VehicleCollision->GetForwardVector().ToOrientationRotator(),
		0.f,
		12.f
	);

	// Update Mass Center
	VehicleCollision->SetCenterOfMass(VehicleMassCenter);
}

void ABaseVehicle::InAirRotation(float DeltaTime)
{
	if (bIsOnGround) {
		return;
	}

	/*VehicleCollision->AddTorqueInDegrees(
		-VehicleCollision->GetRightVector() * Steering.Y * 50.f * 100000000.f);

	VehicleCollision->AddTorqueInRadians(
		FVector(
			0.f,
			0.f,
			Steering.X * 50.f * 100000.f).ProjectOnTo(VehicleCollision->GetUpVector())
	);*/
}

void ABaseVehicle::InstantAccelerationDecrease(float Value)
{
	Acceleration = FMath::Clamp(Acceleration-Value*.0025f, 0.f, 1.f);
	VehicleCollision->AddForceAtLocation(
		-Value * VehicleCollision->GetComponentVelocity() * 5000.f * (1.f-Acceleration),
		VehicleCollision->GetComponentLocation()
	);

	// UE_LOG(LogTemp, Display, TEXT("%f"), VehicleCollision->GetComponentVelocity().Length())
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

bool ABaseVehicle::IsOnGround()
{
	uint8 WheelsOnGround = 0;
	for (UWheelComponent* Wheel : Wheels) {
		if (Wheel->IsOnGround()) {
			WheelsOnGround++;
		}
	}
	return WheelsOnGround == 4;
}

bool ABaseVehicle::IsAnyWheelOnTheGround()
{
	for (UWheelComponent* Wheel : Wheels) {
		if (Wheel->IsOnGround()) {
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

void ABaseVehicle::HydraulicsControlInput(const FInputActionValue& InputValue)
{
	TargetHydraulicsControl = InputValue.Get<FVector2D>();
	OnHydraulicsControlUpdated(TargetHydraulicsControl);
}

void ABaseVehicle::ThrottlePressedInput(const FInputActionValue& InputValue)
{
	UE_LOG(LogTemp, Warning, TEXT("Throttle Pressed."));

	if (MovesetDashTimerHandle.IsValid()){
		DashForward();
	}
}

void ABaseVehicle::ThrottleInput(const FInputActionValue& InputValue)
{
	bool bLastThrottling = bIsThrottling;
	Throttle = FMath::Clamp(InputValue.Get<float>(), -1.0, 1.0);
	bIsThrottling = Throttle > 0.0;

	OnThrottleUpdate(Throttle);
}

void ABaseVehicle::BreakInput(const FInputActionValue& InputValue)
{
	float BreakScale = InputValue.Get<float>();
	bIsBreaking = BreakScale > 0.0;

	InstantAccelerationDecrease(1.0*BreakScale);

	OnBreaking();
}

void ABaseVehicle::GearShiftInput(const FInputActionValue& InputValue)
{
	EGearShift LastShift = CurrentShift;
	bool bLastIsGearShifting = bIsGearShifting;
	bChangedToNewGear = LastShift != CurrentShift;
	float ShiftInput = InputValue.Get<float>();

	if (ShiftInput > 0.0){
		CurrentShift = EGearShift::DRIVE;
	}
	else if (ShiftInput < 0.0){
		CurrentShift = EGearShift::REVERSE;
	}

	bIsGearShifting = (FMath::Abs(ShiftInput) != 0.0);
	if (bLastIsGearShifting != bIsGearShifting){
		OnGearShifting(bIsGearShifting);
		OnGearShiftingDelegate.Broadcast(bIsGearShifting);
	}

	
	if (LastShift != CurrentShift){
		ShiftToNewGear(CurrentShift);	
	}
}

void ABaseVehicle::ShiftToNewGear(EGearShift NewGear)
{
	bJustDashed = false;

	MovesetDashTimerHandle.Invalidate();
	
	GetWorld()->GetTimerManager().SetTimer(
	MovesetDashTimerHandle,
	this, &ABaseVehicle::ClearDashTimer,
		DashTimeWindow, false);
	
	UE_LOG(LogTemp, Warning, TEXT("Start Dash timer!"));

	OnGearShift(NewGear);
	OnGearChangedDelegate.Broadcast(NewGear);
}

void ABaseVehicle::HandbreakInput(const FInputActionValue& InputValue)
{
	bIsHandbreaking = InputValue.Get<bool>();

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

void ABaseVehicle::VehicleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	float ImpactDot = FVector::UpVector.Dot(Hit.ImpactNormal);
	if (!FMath::IsNearlyZero(ImpactDot)) {
		return;
	}

	float ImpactForce = GetVelocity().Length() * VehicleCollision->GetMass() * 50.f;
	VehicleCollision->AddForceAtLocation(
		ImpactForce * Hit.ImpactNormal,
		VehicleCollision->GetComponentLocation()
	);

	UE_LOG(LogTemp, Display, TEXT("Dot: %f, Impact Force: %f"), ImpactDot, ImpactForce);

	UKismetSystemLibrary::DrawDebugArrow(
		this,
		VehicleCollision->GetComponentLocation(),
		VehicleCollision->GetComponentLocation() + ImpactForce,
		10.f,
		FLinearColor::Red,
		10.f,
		1.f
	);

	OnVehicleHit(this, OtherActor, OtherComp, Hit.ImpactPoint, Hit.ImpactNormal);
}

void ABaseVehicle::ClearDashTimer()
{
	MovesetDashTimerHandle.Invalidate();
	UE_LOG(LogTemp, Warning, TEXT("Dash timer cleared!"));
}

void ABaseVehicle::DashForward()
{
	if (!IsAnyWheelOnTheGround() || bJustDashed){
		return;
	}

	bJustDashed = true;
	ClearDashTimer();

	switch (GetCurrentGearShift())
	{
		case EGearShift::DRIVE:
			Acceleration = FMath::Clamp(Acceleration * 1.25f, 0.1f, 1.f);
			UE_LOG(LogTemp, Warning, TEXT("Dash Forward!"));
			break;
		case EGearShift::REVERSE:
			UE_LOG(LogTemp, Display, TEXT("Dash Back! %f"), Acceleration);
			Acceleration = FMath::Clamp(Acceleration * .5f, -1.0f, 0.f);
	}
	
	float TargetDashForce = DashForce * 100000.f;
	VehicleCollision->AddForceAtLocation(
		TargetDashForce * VehicleCollision->GetForwardVector(),
		VehicleCollision->GetComponentLocation()
	);
}

void ABaseVehicle::SetupVehicleWheelComponents() 
{
	for (UActorComponent* ActorWheelComponent : GetComponents()) {
		UWheelComponent* WheelComponent = Cast <UWheelComponent>(ActorWheelComponent);
		if (!WheelComponent) {
			continue;
		}

		Wheels.Add(WheelComponent);

		switch (WheelComponent->GetVerticalAlignment()) {
		case EWheelVerticalAlignment::VA_FRONT:
			FrontWheels.Add(WheelComponent);
			break;
		case EWheelVerticalAlignment::VA_BACK:
			BackWheels.Add(WheelComponent);
			break;
		}

		switch (WheelComponent->GetHorizontalAlignment()) {
		case EWheelHorizontalAlignment::HA_RIGHT:
			RightWheels.Add(WheelComponent);
			break;
		case EWheelHorizontalAlignment::HA_LEFT:
			LeftWheels.Add(WheelComponent);
			break;
		}
	}

	if (Wheels.IsEmpty()) {
		UE_LOG(LogTemp, Error, TEXT("%s Cannot setup vehicle wheels!"), *GetName());
	}
}