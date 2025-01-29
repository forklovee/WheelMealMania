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

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Rig", meta = (AllowPrivateAccess = "true"))
	float TargetHeight = 50.f;
	
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> LookAtTarget;

private:
	FVector DefaultSocketOffset;
	
public:
	UFUNCTION(BlueprintNativeEvent)
	void SetDistanceToTarget(float NewDistance);

	UFUNCTION(BlueprintNativeEvent)
	void SetCameraLookAtTarget(AActor* NewLookAtTarget);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	
	
};
