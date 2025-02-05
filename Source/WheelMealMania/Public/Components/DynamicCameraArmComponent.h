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
	FVector LastDirectionToLookAtTarget = -FVector::ForwardVector;

	bool bFollowTarget = true;
	
public:
	UDynamicCameraArmComponent();

	UFUNCTION(BlueprintCallable)
	float GetDefaultArmLength() const;
	
	UFUNCTION(BlueprintNativeEvent)
	void ResetDistanceToTarget();

	UFUNCTION(BlueprintNativeEvent)
	void SetFollowTargetEnabled(bool bNewFollowTargetState);
	
	UFUNCTION(BlueprintNativeEvent)
	void SetDistanceToTarget(float NewDistance);

	UFUNCTION(BlueprintNativeEvent)
	void SetCameraLookAtTarget(AActor* NewLookAtTarget);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	

};
