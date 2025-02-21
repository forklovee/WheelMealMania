// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/NPCVehicleCrowdController.h"

#include "Kismet/GameplayStatics.h"
#include "Vehicles/NPCVehicle.h"

ANPCVehicleCrowdController::ANPCVehicleCrowdController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANPCVehicleCrowdController::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickInterval(ControllerTickInterval);
	
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(this, ANPCVehicle::StaticClass(), Actors);
	
	for (AActor* VehicleActor : Actors)
	{
		ANPCVehicle* Vehicle = Cast<ANPCVehicle>(VehicleActor);
		if (!Vehicle) continue;
		Vehicles.Add(Vehicle);
		
		Vehicle->SetActorTickEnabled(true);
		Vehicle->SetActorTickInterval(VehicleFarTickInterval);
	}

	PlayerCamera = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;

	UE_LOG(LogTemp, Warning, TEXT("NPC Vehicle Crowd Controller Initialization: \nPlayerCamera: %i, Registered Vehicles=%i"),
		PlayerCamera != nullptr, Vehicles.Num());
	
}

void ANPCVehicleCrowdController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetActorTickInterval(ControllerTickInterval);

	for (ANPCVehicle* Vehicle : Vehicles)
	{
		const float Distance = FVector::Distance(Vehicle->GetActorLocation(), PlayerCamera->GetCameraLocation());

		if (Distance > FarRange && (!IsVehicleVisible(Vehicle) || Distance > FarRange * 2.f))
		{
			Vehicle->SetActorTickEnabled(false);
			Vehicle->SetSimulatePhysics(false);
			Vehicle->SetIsRendering(false);
			continue;
		}
		
		Vehicle->SetActorTickEnabled(true);
		Vehicle->SetIsRendering(true);
		if (Distance > FarRange)
		{
			Vehicle->SetSimulatePhysics(false);
			Vehicle->SetActorTickInterval(VehicleFarTickInterval);
		}
		else
		{
			Vehicle->SetSimulatePhysics(true);
			Vehicle->SetActorTickInterval(VehicleDefaultTickInterval);
		}
	}
}

bool ANPCVehicleCrowdController::IsVehicleOutOfFarRange(ANPCVehicle* Vehicle) const
{
	if (!PlayerCamera.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("NPC Vehicle Crowd Controller: Player Camera is invalid"));
		return true;
	}
	return FVector::Distance(Vehicle->GetActorLocation(), PlayerCamera->GetCameraLocation()) > FarRange;
}

bool ANPCVehicleCrowdController::IsVehicleVisible(ANPCVehicle* Vehicle) const
{
	if (!PlayerCamera.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("NPC Vehicle Crowd Controller: Player Camera is invalid"));
		return false;
	}

	const FVector CameraForward = PlayerCamera->GetActorForwardVector();
	const FVector DirectionToVehicle = (Vehicle->GetActorLocation() - PlayerCamera->GetCameraLocation()).GetSafeNormal();
	
	if (DirectionToVehicle.Dot(CameraForward) < -0.25)
	{
		return false;
	}

	FHitResult Hit;
	UKismetSystemLibrary::LineTraceSingle(
		this,
		PlayerCamera->GetCameraLocation(),
		Vehicle->GetActorLocation(),
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		{Vehicle},
		EDrawDebugTrace::None,
		Hit,
		true);
	return !Hit.bBlockingHit;
}