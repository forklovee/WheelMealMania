// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BaseVehicle.generated.h"

class UBoxComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

UCLASS()
class WHEELMEALMANIA_API ABaseVehicle : public APawn
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float MaxSpeed= 500.f;

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
	float WheelRadius = 5.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float SteeringSensitivity = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	UCurveFloat* SteeringRangeByAcceleration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	UCurveFloat* DriftSteeringRangeByAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float WheelMaxAngleDeg = 55.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brakes")
	float BreakRate = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brakes")
	float HandbreakRate = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float SpringStrength = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float SpringLength = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	FVector MassCenterOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension|Socket")
	float SocketDistanceOffset = 17.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension|Jumping")
	float JumpStrength = 1000.f;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UBoxComponent* VehicleCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* VehicleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraArm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Suspension", meta = (AllowPrivateAccess = "true"))
	USceneComponent* FrontLeftWheelSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Suspension", meta = (AllowPrivateAccess = "true"))
	USceneComponent* FrontLeftSuspensionSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Suspension", meta = (AllowPrivateAccess = "true"))
	USceneComponent* FrontRightWheelSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Suspension", meta = (AllowPrivateAccess = "true"))
	USceneComponent* FrontRightSuspensionSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Suspension", meta = (AllowPrivateAccess = "true"))
	USceneComponent* BackLeftWheelSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Suspension", meta = (AllowPrivateAccess = "true"))
	USceneComponent* BackLeftSuspensionSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Suspension", meta = (AllowPrivateAccess = "true"))
	USceneComponent* BackRightWheelSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Suspension", meta = (AllowPrivateAccess = "true"))
	USceneComponent* BackRightSuspensionSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta = (AllowPrivateAccess="true"))
	UInputAction* ThrottleInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* BreakInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement Actions", meta = (AllowPrivateAccess = "true"))
	UInputAction* HandBreakInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement Actions", meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement Actions", meta = (AllowPrivateAccess = "true"))
	UInputAction* SideBalanceInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SteeringInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Camera", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAroundInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Camera", meta = (AllowPrivateAccess = "true"))
	UInputAction* ResetCameraInputAction;

	UPROPERTY(BlueprintReadOnly, Category="Engine")
	bool bIsThrottling = false;
	UPROPERTY(BlueprintReadOnly, Category = "Engine")
	bool bIsReversing = false;
	UPROPERTY(BlueprintReadOnly, Category = "Breaks")
	bool bIsBreaking = false;
	UPROPERTY(BlueprintReadOnly, Category = "Breaks")
	bool bIsHandbreaking = false;
private:
	// Accelerating
	float Throttle = 0.0;
	float Acceleration = 0.0;

	FVector LastDrivingDirection = FVector::ZeroVector;

	// Steering
	FVector2D TargetSteering = FVector2D::ZeroVector;
	FVector2D Steering = FVector2D::ZeroVector;
	float SteeringRange = 1.f;

	// Side Balance
	float TargetSideBalance = 0.0;

	uint8 JumpCounter = 0;

	bool bIsOnGround = false;
public:
	ABaseVehicle();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	float GetCurrentTargetSpeed();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void ThrottleInput(const FInputActionValue& InputValue);
	UFUNCTION(BlueprintImplementableEvent)
	void OnThrottleUpdate(float NewThrottle);
	UFUNCTION(BlueprintImplementableEvent)
	void OnAccelerationUpdate(float NewAcceleration);

	void SteeringInput(const FInputActionValue& InputValue);
	UFUNCTION(BlueprintImplementableEvent)
	void OnSteeringUpdate(FVector2D NewSteering);

	void BreakInput(const FInputActionValue& InputValue);
	UFUNCTION(BlueprintImplementableEvent)
	void OnBreaking();

	void HandbreakInput(const FInputActionValue& InputValue);
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

private:
	void SuspensionCast(float DeltaTime);
	void UpdateWheelsVelocityAndDirection(float DeltaTime);
	void InAirRotation(float DeltaTime);

	float GetTargetWheelSpeed();
	FVector GetTargetVelocity();

	FVector GetHorizontalVelocity();

	bool WheelCast(USceneComponent* WheelSocket, FHitResult& HitResult);
	bool IsOnGround();
	bool IsAnyWheelOnTheGround();
};
