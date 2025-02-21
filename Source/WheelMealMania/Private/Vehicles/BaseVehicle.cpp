// Fill out your copyright notice in the Description page of Project Settings.

#include "Vehicles/BaseVehicle.h"
#include "Components/BoxComponent.h"
#include "Vehicles/WheelComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"

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
}

// Called when the game starts or when spawned
void ABaseVehicle::BeginPlay()
{
	Super::BeginPlay();

	DrivingDirection = VehicleCollision->GetForwardVector();
	DefaultAngularDamping = VehicleCollision->GetAngularDamping();
	
	SetupVehicleWheelComponents();
	VehicleCollision->OnComponentHit.AddDynamic(this, &ABaseVehicle::VehicleHit);
}

void ABaseVehicle::SetThrottle(float NewThrottle)
{
	Throttle = NewThrottle;
	if (bMovementBlocked)
	{
		Throttle = 0.f;
	}
	bIsThrottling = Throttle > 0.0;
	
	OnThrottleUpdate(Throttle);
}

void ABaseVehicle::SetSteering(FVector2D NewSteering)
{
	TargetSteering = NewSteering;
	OnSteeringUpdate(Steering);
}

void ABaseVehicle::SetBreak(float NewBreak)
{
	if (!bIsOnGround)
	{
		return;
	}
	bIsBreaking = NewBreak > 0.0;

	InstantAccelerationDecrease(0.99999999f);
	
	// const float BreakForce = FMath::Clamp(VehicleCollision->GetComponentVelocity().Length(), 0.f, 500.f); 
	// const FVector VelocityDirection = VehicleCollision->GetComponentVelocity().GetSafeNormal();
	// VehicleCollision->AddForceAtLocation(
	// 	25000.f * BreakForce * NewBreak * -VelocityDirection,
	// 	VehicleCollision->GetComponentLocation());

	// UE_LOG(LogTemp, Display, TEXT("Vehicle breaking: %f"), BreakForce);
	
	OnBreaking();
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
	if (bMovementBlocked)
	{
		TargetSteering = FVector2D::ZeroVector;
	}
	Steering = FMath::Lerp(Steering, TargetSteering,
		(FMath::Abs(TargetSteering.X) > 0.0) ? DeltaTime*SteeringSensitivity : DeltaTime*15.f);

	float SteeringRangeScale = 1.f;
	if (SteeringRangeCurve)
	{
		SteeringRangeScale = SteeringRangeCurve->GetFloatValue(Acceleration);
	}
	SteeringAngle = FMath::Lerp(SteeringAngle, SteeringRangeScale * Steering.X * WheelMaxAngleDeg, DeltaTime*10.0f);
	
	// MAIN PROCESSING
	UpdateAcceleration(GetPhysicsForceDeltaTimeScaler());
	UpdateWheelsVelocityAndDirection(GetPhysicsForceDeltaTimeScaler()); // Updates TargetDriveForce
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

void ABaseVehicle::SetMovementBlocked(bool bNewMovementBlocked)
{
	bMovementBlocked = bNewMovementBlocked;
}

float ABaseVehicle::GetPhysicsForceDeltaTimeScaler() const
{
	return (1.f/24.f) / (GetWorld()->GetDeltaSeconds());
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
	const EGearShift CurrentGear = GetCurrentGearShift();
	
	// Get target acceleration and lerp timescale
	float TargetAcceleration = (CurrentGear == EGearShift::DRIVE) ? 1.f : -1.f;
	if (bMovementBlocked)
	{
		TargetAcceleration = 0.f; 
	}
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

	if (bIsOnGround && FMath::Abs(Throttle) < 0.5f && VehicleCollision->GetComponentVelocity().Length() < 100.f)
	{
		TargetAcceleration *= VehicleCollision->GetComponentVelocity().Length() * 0.01f;
	}
	
	Acceleration = FMath::FInterpTo(Acceleration, TargetAcceleration, GetWorld()->GetDeltaSeconds(), AccelerationLerpTimeScale);

	//Move mass center to... center.
	FVector MaxAccelerationMassCenter = MassCenterOffset;
	MaxAccelerationMassCenter.X = 10.f;
	MaxAccelerationMassCenter.Y = 0.f;
	
	VehicleCollision->SetCenterOfMass(
		(bIsOnGround) ? FMath::Lerp(MassCenterOffset, MaxAccelerationMassCenter, Acceleration) : MaxAccelerationMassCenter
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
	TargetSpeed *= VehicleCollision->GetMass() * 0.25f;

	// Apply speed to wheels
	for (UWheelComponent* Wheel : Wheels) {
		Wheel->Update(TargetSpeed, SteeringAngle);
	}
	
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

void ABaseVehicle::InAir()
{
	FlyDirection = GetVelocity().GetSafeNormal();

	// GetWorldTimerManager().SetTimer(BigAirTimerHandle, this, &ABaseVehicle::UpdateBigAirTrickCounter, 1.f, true, .5f);

	OnInAir();
}

void ABaseVehicle::Landed()
{
	// JumpCounter = 0;
	// YawSpins = 0;
	// CurrentYawSpinDegrees = 0;
	//
	// // Reset Big Air Trick Counter
	// BigAirTrickCounter = 0;
	// GetWorldTimerManager().ClearTimer(BigAirTimerHandle);
			
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
		-Value * VehicleCollision->GetComponentVelocity() * 100.f * (1.f-Acceleration),
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

void ABaseVehicle::ShiftToNewGear(EGearShift NewGear)
{
	OnGearShift(NewGear);
	OnGearChangedDelegate.Broadcast(NewGear);
}

void ABaseVehicle::VehicleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	float ImpactDot = FVector::UpVector.Dot(Hit.ImpactNormal);
	if (!FMath::IsNearlyZero(ImpactDot)) {
		return;
	}

	InstantAccelerationDecrease(.5f);
	
	const float ImpactForce = 100000.f * VehicleCollision->GetMass();
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

void ABaseVehicle::SetupVehicleWheelComponents() 
{
	for (UActorComponent* ActorWheelComponent : GetComponents()) {
		UWheelComponent* WheelComponent = Cast <UWheelComponent>(ActorWheelComponent);
		if (!WheelComponent) {
			continue;
		}
		WheelComponent->Setup(VehicleCollision, WheelMaxAngleDeg, GravityScale, bDrawDebug);
		
		Wheels.Add(WheelComponent);

		// switch (WheelComponent->GetVerticalAlignment()) {
		// case EWheelVerticalAlignment::VA_FRONT:
		// 	FrontWheels.Add(WheelComponent);
		// 	break;
		// case EWheelVerticalAlignment::VA_BACK:
		// 	BackWheels.Add(WheelComponent);
		// 	break;
		// }
		//
		// switch (WheelComponent->GetHorizontalAlignment()) {
		// case EWheelHorizontalAlignment::HA_RIGHT:
		// 	RightWheels.Add(WheelComponent);
		// 	break;
		// case EWheelHorizontalAlignment::HA_LEFT:
		// 	LeftWheels.Add(WheelComponent);
		// 	break;
		// }
	}

	if (Wheels.IsEmpty()) {
		UE_LOG(LogTemp, Error, TEXT("%s Cannot setup vehicle wheels!"), *GetName());
	}
}
