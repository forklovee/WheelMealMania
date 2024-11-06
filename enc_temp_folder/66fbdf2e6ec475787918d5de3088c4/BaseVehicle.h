// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BaseVehicle.generated.h"

class UBoxComponent;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class WHEELMEALMANIA_API ABaseVehicle : public APawn
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float SpringStrength = 500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float SpringLength = 60.f;

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
	USceneComponent* FrontRightWheelSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Suspension", meta = (AllowPrivateAccess = "true"))
	USceneComponent* BackLeftWheelSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Suspension", meta = (AllowPrivateAccess = "true"))
	USceneComponent* BackRightWheelSocket;

public:
	ABaseVehicle();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void SuspensionCast();


};
