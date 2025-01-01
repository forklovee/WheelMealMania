// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WheelComponent.generated.h"

UENUM(BlueprintType)
enum class EWheelVerticalAlignment: uint8 {
	VA_BACK		UMETA(DisplayName = "Back"),
	VA_CENTER	UMETA(DisplayName = "Center"),
	VA_FRONT	UMETA(DisplayName = "Front")
};

UENUM(BlueprintType)
enum class EWheelHorizontalAlignment : uint8 {
	HA_LEFT		UMETA(DisplayName = "Back"),
	HA_RIGHT	UMETA(DisplayName = "Right"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWheelOnGroundSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWheelInAirSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpringLengthUpdatedSignature, float, NewSpringLength);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WHEELMEALMANIA_API UWheelComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnWheelOnGroundSignature OnWheelOnGroundDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnWheelInAirSignature OnWheelInAirDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnSpringLengthUpdatedSignature OnSpringLengthUpdatedDelegate;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel", meta=(AllowPrivateAccess="true"))
	float WheelRadius = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel", meta = (AllowPrivateAccess = "true", DisplayName = "Affected by steering"))
	bool bAffectedBySteering = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel", meta = (AllowPrivateAccess = "true", DisplayName = "Affected by hydraulics"))
	bool bAffectedByHydraulics = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel", meta = (AllowPrivateAccess = "true", EditCondition = "bAffectedByHydraulics"))
	EWheelVerticalAlignment VerticalAlignment = EWheelVerticalAlignment::VA_CENTER;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel", meta = (AllowPrivateAccess = "true", EditCondition = "bAffectedByHydraulics"))
	EWheelHorizontalAlignment HorizontalAlignment = EWheelHorizontalAlignment::HA_LEFT;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel|Suspension", meta = (AllowPrivateAccess = "true"))
	float SpringStrength = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel|Suspension", meta = (AllowPrivateAccess = "true"))
	float SpringLength = 60.f;


private:
	class UBoxComponent* VehicleCollision;

	bool bIsOnGround = false;
	FVector GroundNormal = FVector::UpVector;

	float SpringLengthRatio = 1.f;

	//Target Wheel Height
	float TargetWheelHeight = 0.f;
	//Lerped Wheel Height
	float CurrentWheelHeight = 0.f;
public:	
	// Sets default values for this component's properties
	UWheelComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	inline bool IsAffectedBySteering() { return bAffectedBySteering; };

	UFUNCTION(BlueprintCallable)
	float GetSpringLengthRatio();
	UFUNCTION(BlueprintCallable)
	float GetSpringLength();

	UFUNCTION(BlueprintCallable)
	void SetSpringLengthRatio(float NewSpringLengthRatio);
	UFUNCTION(BlueprintImplementableEvent)
	void OnSpringLengthUpdated(float NewSpringLength);
	
	UFUNCTION(BlueprintCallable)
	inline bool IsAffectedByHydraulics() { return bAffectedByHydraulics; };

	UFUNCTION(BlueprintCallable)
	inline EWheelVerticalAlignment GetVerticalAlignment() { return VerticalAlignment; };
	UFUNCTION(BlueprintCallable)
	inline EWheelHorizontalAlignment GetHorizontalAlignment() { return HorizontalAlignment; };

	UFUNCTION(BlueprintCallable)
	bool IsOnGround();
	UFUNCTION(BlueprintCallable)
	FVector GetGroundNormalVector();
	UFUNCTION(BlueprintImplementableEvent)
	void OnWheelOnGround();
	UFUNCTION(BlueprintImplementableEvent)
	void OnWheelInAir();

protected:
	virtual void BeginPlay() override;

	bool UpdateWheelCollisionCast();
};
