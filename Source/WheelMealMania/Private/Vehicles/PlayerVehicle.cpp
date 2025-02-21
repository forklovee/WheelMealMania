// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/PlayerVehicle.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/DynamicCameraArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/PlayerVehicleController.h"
#include "Vehicles/VehicleSeatComponent.h"
#include "Vehicles/WheelComponent.h"

APlayerVehicle::APlayerVehicle()
{
	CameraArm = CreateDefaultSubobject<UDynamicCameraArmComponent>(FName("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	Camera->SetupAttachment(CameraArm);
}

void APlayerVehicle::BeginPlay()
{
	Super::BeginPlay();

	CameraArm->SetTargetActor(this);
	SetupVehicleSeatComponents();
}

void APlayerVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bMovementBlocked)
	{
		if (!bDriftMode)
		{
			CameraArm->OverrideTargetForwardVector(
				VehicleCollision->GetForwardVector()
			);
		}
		else
		{
			CameraArm->OverrideTargetForwardVector(
				VehicleCollision->GetComponentVelocity().GetSafeNormal()
			);
		}
	}
	
	if (!bIsOnGround)
	{
		InAirRotation(DeltaTime);
	}
}

#pragma region Input

// Called to bind functionality to input
void APlayerVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		APlayerVehicleController* VehicleController = Cast<APlayerVehicleController>(GetController());
		if (VehicleController)
		{
			EnhancedInput->BindAction(PauseMenuInputAction, ETriggerEvent::Triggered, VehicleController, &APlayerVehicleController::TogglePauseMenuInput);
		}
		
		EnhancedInput->BindAction(ThrottleInputAction, ETriggerEvent::Started, this, &APlayerVehicle::ThrottleInputPressed);
		EnhancedInput->BindAction(ThrottleInputAction, ETriggerEvent::Triggered, this, &APlayerVehicle::ThrottleInput);
		EnhancedInput->BindAction(ThrottleInputAction, ETriggerEvent::Completed, this, &APlayerVehicle::ThrottleInput);

		EnhancedInput->BindAction(BreakInputAction, ETriggerEvent::Started, this, &APlayerVehicle::BreakInputPressed);
		EnhancedInput->BindAction(BreakInputAction, ETriggerEvent::Triggered, this, &APlayerVehicle::BreakInput);
		EnhancedInput->BindAction(BreakInputAction, ETriggerEvent::Completed, this, &APlayerVehicle::BreakInput);

		EnhancedInput->BindAction(GearShiftInputAction, ETriggerEvent::Triggered, this, &APlayerVehicle::GearShiftInput);
		EnhancedInput->BindAction(GearShiftInputAction, ETriggerEvent::Completed, this, &APlayerVehicle::GearShiftInput);

		EnhancedInput->BindAction(SteeringInputAction, ETriggerEvent::Started, this, &APlayerVehicle::SteeringInputPressed);
		EnhancedInput->BindAction(SteeringInputAction, ETriggerEvent::Triggered, this, &APlayerVehicle::SteeringInput);
		EnhancedInput->BindAction(SteeringInputAction, ETriggerEvent::Completed, this, &APlayerVehicle::SteeringInput);

		EnhancedInput->BindAction(HydraulicsInputAction, ETriggerEvent::Triggered, this, &APlayerVehicle::HydraulicsControlInput);
		EnhancedInput->BindAction(HydraulicsInputAction, ETriggerEvent::Completed, this, &APlayerVehicle::HydraulicsControlInput);
		
		EnhancedInput->BindAction(JumpInputAction, ETriggerEvent::Started, this, &APlayerVehicle::JumpingInput);
		EnhancedInput->BindAction(JumpInputAction, ETriggerEvent::Triggered, this, &APlayerVehicle::JumpingInput);
		EnhancedInput->BindAction(JumpInputAction, ETriggerEvent::Completed, this, &APlayerVehicle::JumpingInput);
		
		EnhancedInput->BindAction(LookAroundInputAction, ETriggerEvent::Triggered, this, &APlayerVehicle::LookAroundInput);
		EnhancedInput->BindAction(LookAroundInputAction, ETriggerEvent::Completed, this, &APlayerVehicle::LookAroundInput);

		EnhancedInput->BindAction(ResetCameraInputAction, ETriggerEvent::Triggered, this, &APlayerVehicle::ResetCameraInput);
	}
}

void APlayerVehicle::SteeringInput(const FInputActionValue& InputValue)
{
	SetSteering(InputValue.Get<FVector2D>());
}

void APlayerVehicle::SteeringInputPressed(const FInputActionValue& InputValue)
{
	PushKeyToComboBuffer("Steering");
}

void APlayerVehicle::HydraulicsControlInput(const FInputActionValue& InputValue)
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

void APlayerVehicle::ThrottleInput(const FInputActionValue& InputValue)
{
	SetThrottle(InputValue.Get<float>());
}

void APlayerVehicle::ThrottleInputPressed(const FInputActionValue& InputValue)
{
	PushKeyToComboBuffer("Throttle");
}

void APlayerVehicle::BreakInput(const FInputActionValue& InputValue)
{
	SetBreak(InputValue.Get<float>());
}

void APlayerVehicle::BreakInputPressed(const FInputActionValue& InputValue)
{
	PushKeyToComboBuffer("Break");
}

void APlayerVehicle::JumpingInput(const FInputActionValue& InputValue)
{
	if (bMovementBlocked)
	{
		JumpCharge = 0.f;
		for (UWheelComponent* WheelComponent : Wheels)
		{
			WheelComponent->SetSpringStrengthRatio(1.f);
		}
		return;
	}
	
	const float LastJumpCharge = JumpCharge;
	JumpCharge = InputValue.Get<float>();

	UE_LOG(LogTemp, Display, TEXT("Jump! %f"), JumpCharge);
	
	for (UWheelComponent* WheelComponent : Wheels)
	{
		WheelComponent->SetSpringStrengthRatio(1.85f - JumpCharge);
	}

	InstantAccelerationDecrease(0.98);
	
	if (!bIsOnGround || JumpCharge != 0.f)
	{
		return;
	}
	
	VehicleCollision->SetCenterOfMass(FVector::ZeroVector);
	for (UWheelComponent* WheelComponent : Wheels)
	{
		WheelComponent->Jump(LastJumpCharge*JumpStrength*GetPhysicsForceDeltaTimeScaler());
	}
	JumpCounter++;
	for (UWheelComponent* WheelComponent : Wheels)
	{
		WheelComponent->SetSpringStrengthRatio(1.f);
	}
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

void APlayerVehicle::LookAroundInput(const FInputActionValue& InputValue)
{
	FVector2D LookAroundVector = InputValue.Get<FVector2D>() * 10.f;

	CameraArm->AddLocalRotation(FRotator(0.f, LookAroundVector.X, 0.f));
}

void APlayerVehicle::ResetCameraInput(const FInputActionValue& InputValue)
{
	CameraArm->SetRelativeRotation(FRotator::ZeroRotator);
}

void APlayerVehicle::GearShiftInput(const FInputActionValue& InputValue)
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

void APlayerVehicle::ShiftToNewGear(EGearShift NewGear)
{
	Super::ShiftToNewGear(NewGear);

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

#pragma endregion

#pragma region Movesets

void APlayerVehicle::InAirRotation(float DeltaTime)
{
	const float RotationScalar = 1000000.f;
	const float StabilizationScalar = 5050000000.f;

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
		4.0f * RotationScalar * VehicleCollision->GetUpVector() * Steering.X
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

	// const float DirDot = FlyStartDirection.X * VehicleForward.X + FlyStartDirection.Y * VehicleForward.Y;
	// const float DirDet = FlyStartDirection.X * VehicleForward.Y - FlyStartDirection.Y * VehicleForward.X;
	// const float YawSpin = FMath::Atan2(DirDet, DirDot) + PI;
	// CurrentYawSpinDegrees = FMath::RadiansToDegrees(YawSpin);
	//
	// UKismetSystemLibrary::DrawDebugArrow(
	// 	this,
	// 	VehicleCollision->GetComponentLocation(),
	// 	VehicleCollision->GetComponentLocation() + 500.f*FlyStartDirection,
	// 	5.f, FLinearColor::Gray, 0.f, 5.f);
	//
	// UKismetSystemLibrary::DrawDebugArrow(
	// 	this,
	// 	VehicleCollision->GetComponentLocation(),
	// 	VehicleCollision->GetComponentLocation() + 500.f*FlyStartDirection.RotateAngleAxis(CurrentYawSpinDegrees, VehicleCollision->GetUpVector()),
	// 	5.f, FLinearColor::Red, 0.f, 5.f);
	//
	// if ((Steering.X < 0.f && YawSpin < 0.05f) || (Steering.X > 0.f && YawSpin > 2.f*PI - 0.05f))
	// {
	// 	YawSpins++;
	// 	OnTrickPerformed.Broadcast(EVehicleTrick::SPIN, YawSpins);
	// }
	
	// Roll Rotation
	// VehicleCollision->AddTorqueInDegrees(
	// 	3.5f * RotationScalar * VehicleCollision->GetRightVector() * Steering.Y
	// );
}

void APlayerVehicle::DashForward()
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

void APlayerVehicle::ForceBreak()
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

void APlayerVehicle::SetDriftMode(bool bNewDriftMode)
{		
	if (VehicleCollision->GetComponentVelocity().Length() < 500.f)
	{
		bNewDriftMode = false;
	}
	
	bDriftMode = bNewDriftMode;

	for (UWheelComponent* WheelComponent : Wheels)
	{
		WheelComponent->SetDriftMode(bNewDriftMode);
	}
	
	DriftTrickCounter = 0;
	if (bDriftMode)
	{
		InstantAccelerationDecrease(0.98);
		GetWorldTimerManager().SetTimer(DriftTrickTimerHandle, this, &APlayerVehicle::UpdateDriftTrickCounter, 0.5f, true);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(DriftTrickTimerHandle);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Drift Mode Changed to %i"), bNewDriftMode);
}

#pragma endregion

bool APlayerVehicle::HasPassenger(AActor* Passenger) const
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

UVehicleSeatComponent* APlayerVehicle::GetPassengerSeat(AActor* Passenger) const
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

TArray<AActor*> APlayerVehicle::GetPassengers()
{
	TArray<AActor*> Passengers;
	for (const UVehicleSeatComponent* Seat : Seats)
	{
		Passengers.Add(Seat->GetSittingActor());
	}
	return Passengers;
}

UVehicleSeatComponent* APlayerVehicle::GetFirstFreeSeat() const
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

void APlayerVehicle::SetupVehicleSeatComponents()
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

void APlayerVehicle::PushKeyToComboBuffer(FString KeyString)
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

	GetWorld()->GetTimerManager().SetTimer(ComboClearOutTimer, this, &APlayerVehicle::ClearComboBuffer, 0.25f, false);
	
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

void APlayerVehicle::ClearComboBuffer()
{
	GetWorld()->GetTimerManager().ClearTimer(ComboClearOutTimer);

	ComboBuffer = {};
	UE_LOG(LogTemp, Display, TEXT("Clear Buffer."));
}

void APlayerVehicle::UpdateDriftTrickCounter()
{
	DriftTrickCounter++;
	OnTrickPerformed.Broadcast(EVehicleTrick::DRIFT, DriftTrickCounter);
}

void APlayerVehicle::UpdateBigAirTrickCounter()
{
	BigAirTrickCounter++;
	OnTrickPerformed.Broadcast(EVehicleTrick::BIG_AIR, BigAirTrickCounter);
}