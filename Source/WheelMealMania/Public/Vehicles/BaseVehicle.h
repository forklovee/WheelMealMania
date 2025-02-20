// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BaseVehicle.generated.h"

class UBoxComponent;
class UWheelComponent;

UENUM(BlueprintType)
enum class EGearShift : uint8{
	DRIVE = 0		UMETA(DisplayName = "Drive"),
	REVERSE = 1		UMETA(DisplayName = "Reverse")
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGearChanged, EGearShift);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGearShifting, bool);

UCLASS()
class WHEELMEALMANIA_API ABaseVehicle : public APawn
{
	GENERATED_BODY()

public:
	FOnGearChanged OnGearChangedDelegate;
	FOnGearShifting OnGearShiftingDelegate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebug = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float GravityScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float MaxSpeed = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float MaxSpeedOverdrive = 2500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float MaxReverseSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float ThrottleAccelerationRate = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	UCurveFloat* AccelerationRateCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine|Idle")
	float IdleEngineBreakingRate = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine|Idle")
	float InAirBreakingRate = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine|Sound")
	USoundBase* EngineIdleSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine|Sound")
	USoundBase* EngineThrottleSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float SteeringSensitivity = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float SteeringTorqueForce = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	UCurveFloat* SteeringRangeCurve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	UCurveFloat* DriftSteeringRangeByAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float WheelMaxAngleDeg = 75.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brakes")
	float BreakRate = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brakes")
	float HandbreakRate = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	FVector MassCenterOffset = FVector::ZeroVector;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UBoxComponent* VehicleCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* VehicleMesh;

	UPROPERTY(BlueprintReadOnly, Category="Engine")
	bool bIsThrottling = false;
	UPROPERTY(BlueprintReadOnly, Category = "Engine")
	bool bIsReversing = false;
	UPROPERTY(BlueprintReadOnly, Category = "Breaks")
	bool bIsBreaking = false;
	UPROPERTY(BlueprintReadOnly, Category = "Breaks")
	bool bIsHandbreaking = false;
	
	TArray<UWheelComponent*> Wheels;

	// Accelerating
	float Throttle = 0.0;
	bool bDrivingForwards = true;
	float Acceleration = 0.0;
	FVector DrivingDirection = FVector::ZeroVector;
	FVector LastDrivingDirection = FVector::ZeroVector;

	float DefaultAngularDamping = 0.f;
	
	// Gear Shift
	bool bIsGearShifting = false;
	bool bChangedToNewGear = false;
	EGearShift CurrentShift = EGearShift::DRIVE;

	// Steering
	FVector2D TargetSteering = FVector2D::ZeroVector;
	FVector2D Steering = FVector2D::ZeroVector;
	float SteeringAngle = 0.f;
	
	FVector FlyDirection = FVector::ZeroVector;

	bool bIsOnGround = true;
	
public:
	ABaseVehicle();

	virtual void Tick(float DeltaTime) override;

	float GetPhysicsForceDeltaTimeScaler() const;
	
	UFUNCTION(BlueprintCallable, Category = "Driving")
	void SetThrottle(float NewThrottle);
	UFUNCTION(BlueprintCallable, Category = "Driving")
	void SetSteering(FVector2D NewSteering);
	UFUNCTION(BlueprintCallable, Category = "Driving")
	void SetBreak(float NewBreak);

	UFUNCTION(BlueprintCallable, Category = "Ground")
	bool IsOnGround();
	UFUNCTION(BlueprintCallable, Category = "Ground")
	bool IsAnyWheelOnTheGround();
	
	UFUNCTION(BlueprintCallable)
	float GetCurrentTargetSpeed();
	
	UFUNCTION(BlueprintCallable)
	EGearShift GetCurrentGearShift();
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	bool IsThrottling();
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnThrottleUpdate(float NewThrottle);
	UFUNCTION(BlueprintImplementableEvent)
	void OnAccelerationUpdate(float NewAcceleration);
	UFUNCTION(BlueprintImplementableEvent)
	void OnSteeringUpdate(FVector2D NewSteering);
	UFUNCTION(BlueprintImplementableEvent)
	void OnBreaking();
	
	void ShiftToNewGear(EGearShift NewGear);
	UFUNCTION(BlueprintImplementableEvent)
	void OnGearShift(EGearShift NewGear);
	UFUNCTION(BlueprintImplementableEvent)
	void OnGearShifting(bool bNewIsGearShifting);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnInAir();
	UFUNCTION(BlueprintImplementableEvent)
	void OnLanded();

	void InstantAccelerationDecrease(float Value);
	
	UFUNCTION()
	void VehicleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION(BlueprintImplementableEvent)
	void OnVehicleHit(ABaseVehicle* Vehicle,
		AActor* ActorHit, USceneComponent* ComponentHit,
		FVector HitPoint, FVector HitNormal);
	
private:
	void UpdateAcceleration(float DeltaTime);
	void UpdateWheelsVelocityAndDirection(float DeltaTime);
	// void UpdateVehicleSteeringRotation(float DeltaTime);
	
	void InAir();
	void Landed();
	
	float GetTargetWheelSpeed();
	FVector GetTargetVelocity();

	FVector GetHorizontalVelocity();

	void SetupVehicleWheelComponents();
};
