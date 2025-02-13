// Fill out your copyright notice in the Description page of Project Settings.

#include "Vehicles/BaseVehicle.h"

#include "Components/BoxComponent.h"
#include "Camera/CameraComponent.h"

#include "Vehicles/WheelComponent.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"
#include "InputAction.h"

#include <EnhancedInputComponent.h>

#include "Components/DynamicCameraArmComponent.h"
#include "Vehicles/VehicleSeatComponent.h"

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
	
	CameraArm = CreateDefaultSubobject<UDynamicCameraArmComponent>(FName("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	Camera->SetupAttachment(CameraArm);
}

// Called when the game starts or when spawned
void ABaseVehicle::BeginPlay()
{
	Super::BeginPlay();

	CameraArm->SetTargetActor(this);

	DrivingDirection = VehicleCollision->GetForwardVector();
	DefaultAngularDamping = VehicleCollision->GetAngularDamping();
	
	SetupVehicleWheelComponents();
	SetupVehicleSeatComponents();
	VehicleCollision->OnComponentHit.AddDynamic(this, &ABaseVehicle::VehicleHit);
}

// Called every frame
void ABaseVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if the vehicle is grounded.
	bool bNewIsOnGround = IsOnGround();
	if (bNewIsOnGround != bIsOnGround) {
		if (!bNewIsOnGround && IsAnyWheelOnTheGround())
		{
			bNewIsOnGround = true;
		}
		
		if (bNewIsOnGround) {
			Landed();
		}
		else {
			InAir();
		}
	}
	bIsOnGround = bNewIsOnGround;
	
	VehicleCollision->SetEnableGravity(false);

	// Process Steering
	Steering = FMath::Lerp(Steering, TargetSteering,
		(FMath::Abs(TargetSteering.X) > 0.0) ? DeltaTime*SteeringSensitivity : DeltaTime*15.f);

	float SteeringRangeScale = 1.f;
	if (SteeringRangeCurve)
	{
		SteeringRangeScale = SteeringRangeCurve->GetFloatValue(Acceleration);
	}
	SteeringAngle = FMath::Lerp(SteeringAngle, SteeringRangeScale * Steering.X * WheelMaxAngleDeg, DeltaTime*10.0f);
	
	// MAIN PROCESSING
	UpdateAcceleration(DeltaTime);
	// UpdateVehicleSteeringRotation(DeltaTime);
	UpdateWheelsVelocityAndDirection(DeltaTime); // Updates TargetDriveForce

	
	if (!bIsOnGround)
	{
		InAirRotation(DeltaTime);
		CameraArm->OverrideTargetForwardVector(
			(VehicleCollision->GetComponentVelocity() * FVector(1.f, 1.f, 0.f)).GetSafeNormal()
			);
	}
	else
	{
		CameraArm->OverrideTargetForwardVector(GetActorForwardVector());
	}
	// MAIN PROCESSING

	LastDrivingDirection = GetVelocity().GetSafeNormal();
	
	if (bDrawDebug)
	{
		UKismetSystemLibrary::DrawDebugArrow(
			this,
			VehicleCollision->GetComponentLocation(),
			VehicleCollision->GetComponentLocation() + LastDrivingDirection * 300.f,
			120.f,
			FLinearColor::White
		);
	}
}

// Called to bind functionality to input
void ABaseVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInput->BindAction(ThrottleInputAction, ETriggerEvent::Started, this, &ABaseVehicle::ThrottleInputPressed);
		EnhancedInput->BindAction(ThrottleInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::ThrottleInput);
		EnhancedInput->BindAction(ThrottleInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::ThrottleInput);

		EnhancedInput->BindAction(BreakInputAction, ETriggerEvent::Started, this, &ABaseVehicle::BreakInputPressed);
		EnhancedInput->BindAction(BreakInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::BreakInput);
		EnhancedInput->BindAction(BreakInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::BreakInput);

		EnhancedInput->BindAction(GearShiftInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::GearShiftInput);
		EnhancedInput->BindAction(GearShiftInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::GearShiftInput);

		EnhancedInput->BindAction(SteeringInputAction, ETriggerEvent::Started, this, &ABaseVehicle::SteeringInputPressed);
		EnhancedInput->BindAction(SteeringInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::SteeringInput);
		EnhancedInput->BindAction(SteeringInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::SteeringInput);

		EnhancedInput->BindAction(HydraulicsInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::HydraulicsControlInput);
		EnhancedInput->BindAction(HydraulicsInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::HydraulicsControlInput);

		EnhancedInput->BindAction(HandBreakInputAction, ETriggerEvent::Started, this, &ABaseVehicle::HandbreakInputPressed);
		EnhancedInput->BindAction(HandBreakInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::HandbreakInput);
		EnhancedInput->BindAction(HandBreakInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::HandbreakInput);
		
		EnhancedInput->BindAction(JumpInputAction, ETriggerEvent::Started, this, &ABaseVehicle::JumpingInput);
		EnhancedInput->BindAction(JumpInputAction, ETriggerEvent::Triggered, this, &ABaseVehicle::JumpingInput);
		EnhancedInput->BindAction(JumpInputAction, ETriggerEvent::Completed, this, &ABaseVehicle::JumpingInput);
		
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

bool ABaseVehicle::HasPassenger(AActor* Passenger) const
{
	for (const UVehicleSeatComponent* Seat : Seats)
	{
		if (Seat->GetSittingActor() == Passenger)
		{
			return true;
		}
	}
	return false;
}

UVehicleSeatComponent* ABaseVehicle::GetPassengerSeat(AActor* Passenger) const
{
	for (UVehicleSeatComponent* Seat : Seats)
	{
		if (Seat->GetSittingActor() == Passenger)
		{
			return Seat;
		}
	}
	return nullptr;
}

TArray<AActor*> ABaseVehicle::GetPassengers()
{
	TArray<AActor*> Passengers;
	for (const UVehicleSeatComponent* Seat : Seats)
	{
		Passengers.Add(Seat->GetSittingActor());
	}
	return Passengers;
}

UVehicleSeatComponent* ABaseVehicle::GetFirstFreeSeat() const
{
	for (UVehicleSeatComponent* Seat : Seats)
	{
		if (Seat->GetSittingActor() == nullptr)
		{
			return Seat;
		}
	}
	return nullptr;
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
	const EGearShift CurrentGear = GetCurrentGearShift();
	
	// Get target acceleration and lerp timescale
	float TargetAcceleration = (CurrentGear == EGearShift::DRIVE) ? 1.f : -1.f;
	float AccelerationLerpTimeScale = bIsThrottling ? ThrottleAccelerationRate : IdleEngineBreakingRate;
	
	// Set target acceleration to reduce acceleration overdrive
	if (Acceleration > 1.f)
	{
		TargetAcceleration = 1.f;
		AccelerationLerpTimeScale = bIsThrottling ? ThrottleAccelerationRate : 2.f*IdleEngineBreakingRate;
	}

	// Get vehicle driving direction: dot > 0 = forward, dot < 0 = reverse
	// When current gear direction != current vehicle direction, set acceleration to 0 and lerp time to engine breaking rate
	float VehicleDrivingDirection = GetVelocity().GetSafeNormal().Dot(GetActorForwardVector());
	if (FMath::Abs(VehicleDrivingDirection) < 10.0f)
	{
		VehicleDrivingDirection = 0.f;
	}
	
	if ((VehicleDrivingDirection > 0.f && CurrentGear == EGearShift::DRIVE) ||
		(VehicleDrivingDirection < 0.f && CurrentGear == EGearShift::REVERSE))
	{
		TargetAcceleration = 0.f;
		AccelerationLerpTimeScale = IdleEngineBreakingRate;
	}

	// When in air, target acceleration goes to zero, air breaking rate is applied. 
	if (!bIsOnGround)
	{
		TargetAcceleration = 0.f;
		AccelerationLerpTimeScale = InAirBreakingRate;
	}
	TargetAcceleration *= Throttle;
	
	Acceleration = FMath::FInterpTo(Acceleration, TargetAcceleration, DeltaTime, AccelerationLerpTimeScale);

	//Move mass center to... center.
	FVector MaxAccelerationMassCenter = MassCenterOffset;
	MaxAccelerationMassCenter.X = 0.f;
	MaxAccelerationMassCenter.Y = 0.f;
	
	VehicleCollision->SetCenterOfMass(
		FMath::Lerp(MassCenterOffset, MaxAccelerationMassCenter, Acceleration)
	);
}

void ABaseVehicle::UpdateWheelsVelocityAndDirection(float DeltaTime)
{
	float TargetSpeed = (Acceleration > 0.f) ? MaxSpeed : MaxReverseSpeed;
	TargetSpeed *= Acceleration;
	// Acceleration overdrive
	if (Acceleration > 1.f)
	{
		TargetSpeed += MaxSpeedOverdrive * (Acceleration - 1.f);
	}
	TargetSpeed *= VehicleCollision->GetMass();

	// Apply speed to wheels
	for (UWheelComponent* Wheel : Wheels) {
		Wheel->Update(TargetSpeed, SteeringAngle);
	}
	
	// float TorqueForce = TurnAngleScale * SteeringTorqueForce * WheelMaxAngleDeg;
	// if (!bIsOnGround)
	// {
	// 	TorqueForce *= 0.001f;
	// }
	//
	// VehicleCollision->AddTorqueInRadians(
	// 	VehicleCollision->GetUpVector() * Steering.X * TorqueForce * 10000000.f
	// );
	
	if (bDrawDebug)
	{
		UKismetSystemLibrary::DrawDebugArrow(
			this,
			VehicleCollision->GetComponentLocation(),
			VehicleCollision->GetComponentLocation() + GetVelocity(),
			120.f,
			FLinearColor::Red
		);

		UKismetSystemLibrary::DrawDebugBox(
			this,
			VehicleCollision->GetComponentLocation(),
			FVector::OneVector * 50.f,
			FLinearColor::Blue,
			VehicleCollision->GetForwardVector().ToOrientationRotator(),
			0.f,
			25.f
		);
	}
}

void ABaseVehicle::InAirRotation(float DeltaTime)
{
	const float RotationScalar = 1000000000000.f * DeltaTime;
	const float StabilizationScalar = 25000000000.f * DeltaTime;

	const FRotator LocalVehicleRotation = VehicleCollision->GetRelativeRotation();
	const FRotator TargetLocalVehicleRotation = FRotator(0.f, 0.f, 0.f);
	
	// Stabilize
	VehicleCollision->AddTorqueInDegrees(
		StabilizationScalar * (LocalVehicleRotation.Pitch - TargetLocalVehicleRotation.Pitch) * VehicleCollision->GetRightVector()
	);
	VehicleCollision->AddTorqueInDegrees(
		StabilizationScalar * (LocalVehicleRotation.Roll - TargetLocalVehicleRotation.Roll) * VehicleCollision->GetForwardVector()
	);

	// Yaw Rotation
	VehicleCollision->AddTorqueInDegrees(
		8.5f * RotationScalar * VehicleCollision->GetUpVector() * Steering.X
	);

	const FVector VehicleForward = FVector(
		VehicleCollision->GetForwardVector().X,
		VehicleCollision->GetForwardVector().Y,
		0.f
		).GetSafeNormal();
	const FVector FlyStartDirection = FVector(
		FlyDirection.X,
		FlyDirection.Y,
		0.f
		).GetSafeNormal();

	const float DirDot = FlyStartDirection.X * VehicleForward.X + FlyStartDirection.Y * VehicleForward.Y;
	const float DirDet = FlyStartDirection.X * VehicleForward.Y - FlyStartDirection.Y * VehicleForward.X;
	const float YawSpin = FMath::Atan2(DirDet, DirDot) + PI;
	CurrentYawSpinDegrees = FMath::RadiansToDegrees(YawSpin);
	
	UKismetSystemLibrary::DrawDebugArrow(
		this,
		VehicleCollision->GetComponentLocation(),
		VehicleCollision->GetComponentLocation() + 500.f*FlyStartDirection,
		5.f, FLinearColor::Gray, 0.f, 5.f);
	
	UKismetSystemLibrary::DrawDebugArrow(
		this,
		VehicleCollision->GetComponentLocation(),
		VehicleCollision->GetComponentLocation() + 500.f*FlyStartDirection.RotateAngleAxis(CurrentYawSpinDegrees, VehicleCollision->GetUpVector()),
		5.f, FLinearColor::Red, 0.f, 5.f);
	
	if ((Steering.X < 0.f && YawSpin < 0.05f) || (Steering.X > 0.f && YawSpin > 2.f*PI - 0.05f))
	{
		YawSpins++;
		OnTrickPerformed.Broadcast(EVehicleTrick::SPIN, YawSpins);
	}
	
	// Roll Rotation
	// VehicleCollision->AddTorqueInDegrees(
	// 	3.5f * RotationScalar * VehicleCollision->GetRightVector() * Steering.Y
	// );
}

void ABaseVehicle::InAir()
{
	FlyDirection = GetVelocity().GetSafeNormal();

	GetWorldTimerManager().SetTimer(BigAirTimerHandle, this, &ABaseVehicle::UpdateBigAirTrickCounter, 1.f, true, .5f);

	OnInAir();
}

void ABaseVehicle::Landed()
{
	JumpCounter = 0;
	YawSpins = 0;
	CurrentYawSpinDegrees = 0;

	// Reset Big Air Trick Counter
	BigAirTrickCounter = 0;
	GetWorldTimerManager().ClearTimer(BigAirTimerHandle);
			
	OnLanded();
}

void ABaseVehicle::InstantAccelerationDecrease(float Value)
{
	if (Acceleration < 0.f)
	{
		Value = -Value;
	}
	
	Acceleration = FMath::Clamp(Acceleration-Value*.005f, -1.f, 2.f);
	
	VehicleCollision->AddForceAtLocation(
		-Value * VehicleCollision->GetComponentVelocity() * 5000.f * (1.f-Acceleration),
		VehicleCollision->GetComponentLocation()
	);
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

void ABaseVehicle::SteeringInputPressed(const FInputActionValue& InputValue)
{
	PushKeyToComboBuffer("Steering");
}

void ABaseVehicle::HydraulicsControlInput(const FInputActionValue& InputValue)
{
	TargetHydraulicsControl = InputValue.Get<float>();
	return;
	
	// Reset spring down vector
	for (UWheelComponent* Wheel : Wheels)
	{
		Wheel->SetSpringPointingDown(false);
	}

	const FVector TargetSideCenter = VehicleCollision->GetCenterOfMass()
		+ TargetHydraulicsControl * (VehicleCollision->GetRightVector()*FVector(1.f, 1.f, 0.f)).GetSafeNormal() * 100.f
		+ FVector::UpVector * 135.f;
	const FVector SideCenter = VehicleCollision->GetCenterOfMass()
		+ TargetHydraulicsControl * VehicleCollision->GetRightVector() * 100.f;

	TArray<UWheelComponent*>* WheelsToRaise = (TargetHydraulicsControl > 0.f ) ? &RightWheels : &LeftWheels;
	TArray<UWheelComponent*>* WheelsOnGround = (TargetHydraulicsControl > 0.f ) ? &LeftWheels : &RightWheels;
	
	UKismetSystemLibrary::DrawDebugBox(
		this,
		SideCenter,
		FVector::OneVector * 15.f,
		FLinearColor::Red,
		VehicleCollision->GetComponentRotation(),
		0.f,
		15.f);

	UKismetSystemLibrary::DrawDebugBox(
		this,
		TargetSideCenter,
		FVector::OneVector * 15.f,
		FLinearColor::Blue,
		VehicleCollision->GetComponentRotation(),
		0.f,
		15.f);

	const FVector BalanceForce = (TargetSideCenter - SideCenter) * JumpStrength * 5.f;

	uint8 IWheelsOnGround = 0;
	for (UWheelComponent* Wheel : *WheelsOnGround)
	{
		if (Wheel->IsOnGround())
		{
			IWheelsOnGround++;
		}
	}
	
	if (IWheelsOnGround < 2)
	{
		for (UWheelComponent* Wheel : *WheelsOnGround)
		{
			Wheel->SetSpringPointingDown(false);
		}
		return;
	}

	for (UWheelComponent* Wheel : *WheelsOnGround)
	{
		Wheel->SetSpringPointingDown(true);
		// VehicleCollision->AddForceAtLocation(
		// 	-BalanceForce * .25f,
		// 	Wheel->GetComponentLocation());
	}
	
	for (UWheelComponent* Wheel : *WheelsToRaise)
	{
		Wheel->SetSpringPointingDown(false);
		
		VehicleCollision->AddForceAtLocation(
			BalanceForce,
			Wheel->GetComponentLocation());
	}
	
	OnHydraulicsControlUpdated(TargetHydraulicsControl);
}

void ABaseVehicle::ThrottleInput(const FInputActionValue& InputValue)
{
	Throttle = InputValue.Get<float>();
	bIsThrottling = Throttle > 0.0;
	
	OnThrottleUpdate(Throttle);
}

void ABaseVehicle::ThrottleInputPressed(const FInputActionValue& InputValue)
{
	PushKeyToComboBuffer("Throttle");
}

void ABaseVehicle::BreakInput(const FInputActionValue& InputValue)
{
	float BreakScale = InputValue.Get<float>();
	bIsBreaking = BreakScale > 0.0;

	InstantAccelerationDecrease(1.0*BreakScale);

	OnBreaking();
}

void ABaseVehicle::BreakInputPressed(const FInputActionValue& InputValue)
{
	PushKeyToComboBuffer("Break");
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

	if (bDriftMode && bIsGearShifting != bLastIsGearShifting)
	{
		SetDriftMode(false);
	}
}

void ABaseVehicle::ShiftToNewGear(EGearShift NewGear)
{
	OnGearShift(NewGear);
	OnGearChangedDelegate.Broadcast(NewGear);

	switch (NewGear)
	{
		case EGearShift::DRIVE:
			CameraArm->SetDistanceToTarget(CameraArm->GetDefaultArmLength());
			PushKeyToComboBuffer("Drive");
			break;
		case EGearShift::REVERSE:
			CameraArm->SetDistanceToTarget(CameraArm->GetDefaultArmLength()*1.05f);
			PushKeyToComboBuffer("Reverse");
			break;
	}
}

void ABaseVehicle::HandbreakInput(const FInputActionValue& InputValue)
{
	bIsHandbreaking = InputValue.Get<bool>();

	OnHandbreaking();
}

void ABaseVehicle::HandbreakInputPressed(const FInputActionValue& InputValue)
{
	PushKeyToComboBuffer("Break");
}

void ABaseVehicle::JumpingInput(const FInputActionValue& InputValue)
{
	const float LastJumpCharge = JumpCharge;
	JumpCharge = InputValue.Get<float>();

	UE_LOG(LogTemp, Display, TEXT("Jump! %f"), JumpCharge);
	
	for (UWheelComponent* WheelComponent : Wheels)
	{
		WheelComponent->SetSpringStrengthRatio(1.85f - JumpCharge);
	}
	
	if (!bIsOnGround || JumpCharge != 0.f)
	{
		return;
	}

	for (UWheelComponent* WheelComponent : Wheels)
	{
		WheelComponent->Jump(LastJumpCharge*JumpStrength);
	}
	JumpCounter++;

	// UE_LOG(LogTemp, Display, TEXT("Jump! "));
	//
	// // Jump!
	//
	// const float TargetJumpStrength = LastJumpCharge * JumpStrength * VehicleCollision->GetMass() * 100.f;
	// const FVector JumpForce = VehicleCollision->GetUpVector() * TargetJumpStrength;
	// VehicleCollision->AddForceAtLocation(
	// 	JumpForce,
	// 	VehicleCollision->GetComponentLocation()
	// );

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

	Acceleration *= 0.25;
	
	const float ImpactForce = 5000.f * VehicleCollision->GetMass();
	VehicleCollision->AddForceAtLocation(
		ImpactForce * Hit.ImpactNormal,
		VehicleCollision->GetComponentLocation()
	);

	UE_LOG(LogTemp, Display, TEXT("Dot: %f, Impact Force: %f"), ImpactDot, ImpactForce);

	if (bDrawDebug)
	{
		UKismetSystemLibrary::DrawDebugArrow(
			this,
			VehicleCollision->GetComponentLocation(),
			VehicleCollision->GetComponentLocation() + ImpactForce,
			10.f,
			FLinearColor::Red,
			10.f,
			1.f
		);
	}
	
	OnVehicleHit(this, OtherActor, OtherComp, Hit.ImpactPoint, Hit.ImpactNormal);
}

void ABaseVehicle::DashForward()
{
	if (!IsAnyWheelOnTheGround()){
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Dash Forward!"));
	
	Acceleration = FMath::Clamp(Acceleration + DashAccelerationBoost, -1.0f, 2.f);
	
	const float TargetDashForce = DashForce * 15000.f;
	VehicleCollision->AddForceAtLocation(
		TargetDashForce * VehicleCollision->GetForwardVector(),
		VehicleCollision->GetComponentLocation()
	);
}

void ABaseVehicle::ForceBreak()
{
	if (GetVelocity().Length() > 500.f)
	{
		const float TargetDashForce = DashForce * 15000.f * Acceleration;
		VehicleCollision->AddForceAtLocation(
			TargetDashForce * -VehicleCollision->GetForwardVector(),
			VehicleCollision->GetComponentLocation()
		);
	}
	
	Acceleration *= 0.5;
}

void ABaseVehicle::SetDriftMode(bool bNewDriftMode)
{
	bDriftMode = bNewDriftMode;

	for (UWheelComponent* WheelComponent : Wheels)
	{
		WheelComponent->SetDriftMode(bNewDriftMode);
	}
	
	DriftTrickCounter = 0;
	if (bDriftMode)
	{
		GetWorldTimerManager().SetTimer(DriftTrickTimerHandle, this, &ABaseVehicle::UpdateDriftTrickCounter, 0.5f, true);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(DriftTrickTimerHandle);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Drift Mode Changed to %i"), bNewDriftMode);
}

void ABaseVehicle::SetupVehicleWheelComponents() 
{
	for (UActorComponent* ActorWheelComponent : GetComponents()) {
		UWheelComponent* WheelComponent = Cast <UWheelComponent>(ActorWheelComponent);
		if (!WheelComponent) {
			continue;
		}
		WheelComponent->Setup(VehicleCollision, WheelMaxAngleDeg, GravityScale, bDrawDebug);
		
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

void ABaseVehicle::SetupVehicleSeatComponents()
{
	for (UActorComponent* ActorSeatComponent : GetComponents())
	{
		UVehicleSeatComponent* SeatComponent = Cast<UVehicleSeatComponent>(ActorSeatComponent);
		if (!SeatComponent)
		{
			continue;
		}
		Seats.Add(SeatComponent);
	}
}

void ABaseVehicle::PushKeyToComboBuffer(FString KeyString)
{
	GetWorld()->GetTimerManager().ClearTimer(ComboClearOutTimer);
	
	ComboBuffer.Push(KeyString);
	if (ComboBuffer.Num() > 3)
	{
		ComboBuffer.RemoveAt(ComboBuffer.Num() - 1);
	}
	FString ComboBufferString = "";
	for (FString ComboKeyName : ComboBuffer)
	{
		ComboBufferString += ComboKeyName+"|";
	}
	UE_LOG(LogTemp, Display, TEXT("Buffer %s"), *ComboBufferString);

	GetWorld()->GetTimerManager().SetTimer(ComboClearOutTimer, this, &ABaseVehicle::ClearComboBuffer, 0.25f, false);
	
	if (ComboBuffer.Num() < 1)
	{
		return;
	}

	//Dash
	if (ComboBufferString.Contains("Drive|Throttle|"))
	{
		UE_LOG(LogTemp, Display, TEXT("Dash!"));
		DashForward();
		ClearComboBuffer();
		return;
	}
	
	// Force Break
	if (ComboBufferString.Contains("Reverse|Break|") ||
		ComboBufferString.Contains("Break|Reverse|"))
	{
		ForceBreak();
		ClearComboBuffer();
		return;
	}

	// Drift
	if (bIsThrottling && (
		ComboBufferString.Contains("Reverse|Drive|") ||
		ComboBufferString.Contains("Reverse|Steering|Drive|"))
		)
	{
		SetDriftMode(true);
	}
}

void ABaseVehicle::ClearComboBuffer()
{
	GetWorld()->GetTimerManager().ClearTimer(ComboClearOutTimer);

	ComboBuffer = {};
	UE_LOG(LogTemp, Display, TEXT("Clear Buffer."));
}

void ABaseVehicle::UpdateDriftTrickCounter()
{
	DriftTrickCounter++;
	OnTrickPerformed.Broadcast(EVehicleTrick::DRIFT, DriftTrickCounter);
}

void ABaseVehicle::UpdateBigAirTrickCounter()
{
	BigAirTrickCounter++;
	OnTrickPerformed.Broadcast(EVehicleTrick::BIG_AIR, BigAirTrickCounter);
}
