// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"

#include "Vehicles/BaseVehicle.h"

// Sets default values
ABaseVehicle::ABaseVehicle()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VehicleCollision = CreateDefaultSubobject<UBoxComponent>(FName("CollisionBox"));
	VehicleCollision->SetLinearDamping(2.f);
	RootComponent = VehicleCollision;
	
	VehicleMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("VehicleMesh"));
	VehicleMesh->SetupAttachment(RootComponent);
	
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(FName("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	Camera->SetupAttachment(CameraArm);

	//Setup Suspension Sockets
	FrontLeftWheelSocket = CreateDefaultSubobject<USceneComponent>(FName("FrontLeftWheelSocket"));
	FrontLeftWheelSocket->SetupAttachment(RootComponent);
	FrontLeftWheelSocket->SetRelativeLocation(FVector(50.f, -20.f, 0.f));
	FrontRightWheelSocket = CreateDefaultSubobject<USceneComponent>(FName("FrontRightWheelSocket"));
	FrontRightWheelSocket->SetupAttachment(RootComponent);
	FrontRightWheelSocket->SetRelativeLocation(FVector(50.f, 20.f, 0.f));
	BackLeftWheelSocket = CreateDefaultSubobject<USceneComponent>(FName("BackLeftWheelSocket"));
	BackLeftWheelSocket->SetupAttachment(RootComponent);
	BackLeftWheelSocket->SetRelativeLocation(FVector(-50.f, -20.f, 0.f));
	BackRightWheelSocket = CreateDefaultSubobject<USceneComponent>(FName("BackRightWheelSocket"));
	BackRightWheelSocket->SetupAttachment(RootComponent);
	BackRightWheelSocket->SetRelativeLocation(FVector(-50.f, 20.f, 0.f));

}

// Called when the game starts or when spawned
void ABaseVehicle::BeginPlay()
{
	Super::BeginPlay();
	
}



// Called every frame
void ABaseVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SuspensionCast();
}

// Called to bind functionality to input
void ABaseVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseVehicle::SuspensionCast()
{
	for (USceneComponent* WheelSocket : { FrontLeftWheelSocket, FrontRightWheelSocket, BackLeftWheelSocket, BackRightWheelSocket }) {
		FHitResult TraceHitResult;
		UKismetSystemLibrary::SphereTraceSingle(
			this,
			WheelSocket->GetComponentLocation(),
			WheelSocket->GetComponentLocation() + FVector(0.f, 0.f, -SpringLength),
			15.f,
			UEngineTypes::ConvertToTraceType(ECC_Visibility),
			false,
			{this},
			EDrawDebugTrace::ForOneFrame,
			TraceHitResult,
			true,
			FLinearColor::Blue,
			FLinearColor::Red,
			0.1f
		);

		if (!TraceHitResult.bBlockingHit) {
			continue;
		}
		
		float DistanceNormalized = FMath::GetMappedRangeValueClamped(FVector2D(0.0, SpringLength), FVector2D(0.0, 1.0), TraceHitResult.Distance);
		float DistanceInversed = 1.f - DistanceNormalized;

		UE_LOG(LogTemp, Display, TEXT("Dist: %f Norm: %f Inv: %f"), TraceHitResult.Distance, DistanceNormalized, DistanceInversed);

		FVector TraceOffset = TraceHitResult.TraceStart - TraceHitResult.TraceEnd;
		FVector TraceDirection = TraceOffset / FVector::Distance(TraceHitResult.TraceStart, TraceHitResult.TraceEnd);
		FVector TargetForce = DistanceInversed * VehicleCollision->GetMass() * SpringStrength * TraceDirection;
		VehicleCollision->AddForceAtLocation(
			TargetForce,
			WheelSocket->GetComponentLocation()
		);

		UE_LOG(LogTemp, Display, TEXT("TargetForce: %s"), *TargetForce.ToString());
	}
}