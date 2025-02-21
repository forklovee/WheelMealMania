// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Vehicles/BaseVehicle.h"
#include "PlayerVehicle.generated.h"

class UDynamicCameraArmComponent;
class UVehicleSeatComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

UENUM(BlueprintType)
enum class EVehicleTrick : uint8
{
	NEAR_MISS = 0	UMETA(DisplayName = "Near Miss"),
	BIG_AIR = 1	UMETA(DisplayName = "Big Air"),
	SPIN = 3	UMETA(DisplayName = "Spin"),
	DRIFT = 4	UMETA(DisplayName = "Drift"),
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrickPerformed, EVehicleTrick, VehicleTrick, int, SubsuquentialTricks);

/**
 * 
 */
UCLASS()
class WHEELMEALMANIA_API APlayerVehicle : public ABaseVehicle
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnTrickPerformed OnTrickPerformed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movesets")
	float DashAccelerationBoost = 0.45f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movesets")
	float DashForce = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movesets")
	float DriftingMaxAngleDeg = 45.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movesets")
	float JumpStrength = 1000.f;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UDynamicCameraArmComponent* CameraArm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

#pragma region InputActions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|UI", meta = (AllowPrivateAccess="true"))
	UInputAction* PauseMenuInputAction;
	
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
	UInputAction* HydraulicsInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SteeringInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Camera", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAroundInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Camera", meta = (AllowPrivateAccess = "true"))
	UInputAction* ResetCameraInputAction;

#pragma endregion

private:
	TArray<UVehicleSeatComponent*> Seats;

	TArray<UWheelComponent*> FrontWheels;
	TArray<UWheelComponent*> BackWheels;
	TArray<UWheelComponent*> LeftWheels;
	TArray<UWheelComponent*> RightWheels;

	uint8 YawSpins = 0;
	float CurrentYawSpinDegrees = 0.f;

	// Rotation Control
	float TargetHydraulicsControl = 0.f;
	float HydraulicsControl = 0.f;

	uint8 JumpCounter = 0;

	// Movesets
	TArray<FMovesetComboKeys> ComboKeys;
	TArray<FString> ComboBuffer;

	// Jump
	float JumpCharge = 0.f;
	
	// Drift mode
	bool bDriftMode = false;
	float DriftAngle = 0.f;
	float LastDriftAngle = 0.f;

	// Tricks
	FTimerHandle DriftTrickTimerHandle;
	uint8 DriftTrickCounter = 0;
	FTimerHandle BigAirTimerHandle;
	uint8 BigAirTrickCounter = 0;
	
	FTimerHandle ComboClearOutTimer;

public:
	APlayerVehicle();

#pragma region Passangers
	UFUNCTION(BlueprintCallable)
	bool HasPassenger(AActor* Passenger) const;
	UFUNCTION(BlueprintCallable)
	UVehicleSeatComponent* GetPassengerSeat(AActor* Passenger) const;
	UFUNCTION(BlueprintCallable)
	TArray<AActor*> GetPassengers();
	UFUNCTION(BlueprintCallable)
	UVehicleSeatComponent* GetFirstFreeSeat() const;
#pragma endregion
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

#pragma region Input
	void ThrottleInput(const FInputActionValue& InputValue);
	void ThrottleInputPressed(const FInputActionValue& InputValue);
	void SteeringInput(const FInputActionValue& InputValue);
	void SteeringInputPressed(const FInputActionValue& InputValue);
	void BreakInput(const FInputActionValue& InputValue);
	void BreakInputPressed(const FInputActionValue& InputValue);

	void LookAroundInput(const FInputActionValue& InputValue);
	void ResetCameraInput(const FInputActionValue& InputValue);
	
	void GearShiftInput(const FInputActionValue& InputValue);
	virtual void ShiftToNewGear(EGearShift NewGear) override;
	
	void JumpingInput(const FInputActionValue& InputValue);
	UFUNCTION(BlueprintImplementableEvent)
	void OnJumped();
	void HydraulicsControlInput(const FInputActionValue& InputValue);
	UFUNCTION(BlueprintImplementableEvent)
	void OnHydraulicsControlUpdated(float NewHydraulicsControl);
#pragma endregion

#pragma region Movesets
	void InAirRotation(float DeltaTime);
	void DashForward();
	void ForceBreak();
	void SetDriftMode(bool bNewDriftMode);
#pragma endregion

private:
	void SetupVehicleSeatComponents();

	void PushKeyToComboBuffer(FString KeyString);
	UFUNCTION()
	void ClearComboBuffer();

	// Tricks
	void UpdateDriftTrickCounter();
	void UpdateBigAirTrickCounter();
};
