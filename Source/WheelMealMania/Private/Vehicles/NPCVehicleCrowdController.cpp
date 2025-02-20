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
		Vehicle->SetHidden(false);
		if (IsVehicleOutOfFarRange(Vehicle))
		{
			if (!IsVehicleVisible(Vehicle))
			{
				Vehicle->SetActorTickEnabled(false);
				Vehicle->SetHidden(true);
				continue;
			}
			Vehicle->SetActorTickEnabled(true);
			Vehicle->SetActorTickInterval(VehicleFarTickInterval);
		}
		else
		{
			Vehicle->SetActorTickEnabled(true);
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
	const FVector DirectionToVehicle = (Vehicle->GetActorLocation() - PlayerCamera->GetCameraLocation());

	UKismetSystemLibrary::DrawDebugBox(
	this,
	PlayerCamera->GetCameraLocation(),
	PlayerCamera->GetCameraLocation() + CameraForward * 100000.f,
	FLinearColor::Red,
	FRotator::ZeroRotator,
	0.01f,
	10.f);

	
	if (DirectionToVehicle.Dot(CameraForward) < -0.25)
	{
		return false;
	}
	
	FHitResult Hit;
	UKismetSystemLibrary::LineTraceSingle(
		this,
		PlayerCamera->GetCameraLocation(),
		PlayerCamera->GetCameraLocation() + PlayerCamera->GetActorForwardVector() * 100000.f,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		{},
		EDrawDebugTrace::ForOneFrame,
		Hit,
		true);
	return Hit.bBlockingHit;
}