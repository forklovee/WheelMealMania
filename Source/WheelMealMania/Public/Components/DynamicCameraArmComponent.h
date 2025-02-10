// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "DynamicCameraArmComponent.generated.h"

/**
 * 
 */
UCLASS()
class WHEELMEALMANIA_API UDynamicCameraArmComponent : public USpringArmComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAffectYaw = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAffectPitch = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAffectRoll = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Rig", meta = (AllowPrivateAccess = "true"))
	float TargetHeight = 50.f;

protected:
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> LookAtTarget;

private:
	float DefaultArmLength = 0.f;
	//Target of the target :))))
	float TargetTargetArmLength = 0.f;
	FVector DefaultSocketOffset = FVector::ZeroVector;

	FVector TargetForwardVectorOverride = FVector::ZeroVector;
	FVector LastTargetForwardVector = -FVector::ForwardVector;

	bool bFollowTarget = true;
	
public:
	UDynamicCameraArmComponent();
	
	UFUNCTION(BlueprintCallable)
	void SetTargetActor(AActor* NewLookAtTarget);

	UFUNCTION(BlueprintCallable)
	float GetDefaultArmLength() const;
	UFUNCTION(BlueprintCallable)
	void SetDistanceToTarget(float NewDistance);
	UFUNCTION(BlueprintCallable)
	void ResetDistanceToTarget();
	
	UFUNCTION(BlueprintCallable)
	void SetFollowTargetEnabled(bool bNewFollowTargetState);
	UFUNCTION(BlueprintCallable)
	void OverrideTargetForwardVector(FVector TargetForwardVector);
	UFUNCTION(BlueprintCallable)
	void ClearTargetForwardVectorOverride();
	UFUNCTION(BlueprintCallable)
	bool IsTargetForwardVectorOverriden() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	

private:
	void UpdateSocketHeight(float DeltaTime);
	void FollowTargetForwardVector(float DeltaTime);
};
