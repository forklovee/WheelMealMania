// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BaseVehicle.generated.h"

class UDynamicCameraArmComponent;
class UVehicleSeatComponent;
class UBoxComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UNiagaraSystem;
class UNiagaraComponent;
class UWheelComponent;
struct FInputActionValue;

UENUM(BlueprintType)
enum class EGearShift : uint8{
	DRIVE = 0		UMETA(DisplayName = "Drive"),
	REVERSE = 1		UMETA(DisplayName = "Reverse")
};

USTRUCT(BlueprintType)
struct FMovesetComboKeys
{
	GENERATED_BODY()

	FMovesetComboKeys() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ComboName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Keys;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float SocketDistanceOffset = 17.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movesets|Dash")
	float DashAccelerationBoost = 0.45f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movesets|Dash")
	float DashForce = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movesets|Drifting")
	float DriftingMaxAngleDeg = 45.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movesets|Jumping")
	float JumpStrength = 1000.f;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UBoxComponent* VehicleCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* VehicleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UDynamicCameraArmComponent* CameraArm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

#pragma region InputActions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta = (AllowPrivateAccess="true"))
	UInputAction* ThrottleInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* BreakInputAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* GearShiftInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement Actions", meta = (AllowPrivateAccess = "true"))
	UInputAction* HandBreakInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement Actions", meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement Actions", meta = (AllowPrivateAccess = "true"))
	UInputAction* RotationControlInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SteeringInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Camera", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAroundInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Camera", meta = (AllowPrivateAccess = "true"))
	UInputAction* ResetCameraInputAction;

#pragma endregion InputActions

	UPROPERTY(BlueprintReadOnly, Category="Engine")
	bool bIsThrottling = false;
	UPROPERTY(BlueprintReadOnly, Category = "Engine")
	bool bIsReversing = false;
	UPROPERTY(BlueprintReadOnly, Category = "Breaks")
	bool bIsBreaking = false;
	UPROPERTY(BlueprintReadOnly, Category = "Breaks")
	bool bIsHandbreaking = false;

private:
	TArray<UVehicleSeatComponent*> Seats;
	TArray<UWheelComponent*> Wheels;

	TArray<UWheelComponent*> FrontWheels;
	TArray<UWheelComponent*> BackWheels;
	TArray<UWheelComponent*> LeftWheels;
	TArray<UWheelComponent*> RightWheels;

	TMap<USceneComponent*, UNiagaraComponent*> WheelSocketTrails;

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
	
	FVector2D DriftSteering = FVector2D::ZeroVector;
	float SteeringRange = 1.f;

	// Rotation Control
	FVector2D TargetHydraulicsControl = FVector2D::ZeroVector;
	FVector2D HydraulicsControl = FVector2D::ZeroVector;

	// Side Balance
	float TargetSideBalance = 0.0;

	uint8 JumpCounter = 0;

	bool bIsOnGround = false;

	// Movesets
	TArray<FMovesetComboKeys> ComboKeys;
	TArray<FString> ComboBuffer;

	// Drift mode
	bool bDriftMode = false;
	float DriftAngle = 0.f;
	float LastDriftAngle = 0.f;
	
	FTimerHandle ComboClearOutTimer;
	
public:
	ABaseVehicle();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	float GetCurrentTargetSpeed();
	
	UFUNCTION(BlueprintCallable)
	EGearShift GetCurrentGearShift();

	UFUNCTION(BlueprintCallable)
	bool HasPassenger(AActor* Passenger) const;

	UFUNCTION(BlueprintCallable)
	UVehicleSeatComponent* GetPassengerSeat(AActor* Passenger) const;
	
	UFUNCTION(BlueprintCallable)
	TArray<AActor*> GetPassengers();

	UFUNCTION(BlueprintCallable)
	UVehicleSeatComponent* GetFirstFreeSeat() const;
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	bool IsThrottling();

	void ThrottleInput(const FInputActionValue& InputValue);
	void ThrottleInputPressed(const FInputActionValue& InputValue);
	UFUNCTION(BlueprintImplementableEvent)
	void OnThrottleUpdate(float NewThrottle);
	UFUNCTION(BlueprintImplementableEvent)
	void OnAccelerationUpdate(float NewAcceleration);

	void SteeringInput(const FInputActionValue& InputValue);
	void SteeringInputPressed(const FInputActionValue& InputValue);
	UFUNCTION(BlueprintImplementableEvent)
	void OnSteeringUpdate(FVector2D NewSteering);

	void HydraulicsControlInput(const FInputActionValue& InputValue);
	UFUNCTION(BlueprintImplementableEvent)
	void OnHydraulicsControlUpdated(FVector2D NewHydraulicsControl);

	void BreakInput(const FInputActionValue& InputValue);
	void BreakInputPressed(const FInputActionValue& InputValue);
	UFUNCTION(BlueprintImplementableEvent)
	void OnBreaking();

	void GearShiftInput(const FInputActionValue& InputValue);
	void ShiftToNewGear(EGearShift NewGear);
	UFUNCTION(BlueprintImplementableEvent)
	void OnGearShift(EGearShift NewGear);
	UFUNCTION(BlueprintImplementableEvent)
	void OnGearShifting(bool bNewIsGearShifting);

	void HandbreakInput(const FInputActionValue& InputValue);
	void HandbreakInputPressed(const FInputActionValue& InputValue);
	UFUNCTION(BlueprintImplementableEvent)
	void OnHandbreaking();

	void JumpingInput(const FInputActionValue& InputValue);
	UFUNCTION(BlueprintImplementableEvent)
	void OnJumped();
	UFUNCTION(BlueprintImplementableEvent)
	void OnInAir();
	UFUNCTION(BlueprintImplementableEvent)
	void OnLanded();

	void SideBalanceInput(const FInputActionValue& InputValue);
	UFUNCTION(BlueprintImplementableEvent)
	void OnSideBalanceChanged(float NewSideBalance);

	void LookAroundInput(const FInputActionValue& InputValue);
	void ResetCameraInput(const FInputActionValue& InputValue);

	UFUNCTION()
	void VehicleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION(BlueprintImplementableEvent)
	void OnVehicleHit(ABaseVehicle* Vehicle,
		AActor* ActorHit, USceneComponent* ComponentHit,
		FVector HitPoint, FVector HitNormal);

#pragma region Movesets
	void DashForward();
	void ForceBreak();
	void SetDriftMode(bool bNewDriftMode);
#pragma endregion
	
private:
	void UpdateAcceleration(float DeltaTime);
	void UpdateWheelsVelocityAndDirection(float DeltaTime);
	// void UpdateVehicleSteeringRotation(float DeltaTime);
	
	void InAirRotation(float DeltaTime);

	void InstantAccelerationDecrease(float Value);

	float GetTargetWheelSpeed();
	FVector GetTargetVelocity();

	FVector GetHorizontalVelocity();

	bool IsOnGround();
	bool IsAnyWheelOnTheGround();

	void SetupVehicleWheelComponents();
	void SetupVehicleSeatComponents();

	void PushKeyToComboBuffer(FString KeyString);
	UFUNCTION()
	void ClearComboBuffer();
};
