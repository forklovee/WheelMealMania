// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerVehicleController.h"

#include "EnhancedInputComponent.h"
#include "UI/PlayerHUD.h"
#include "Vehicles/BaseVehicle.h"
#include "EnhancedInputSubsystems.h"

UPlayerHUD* APlayerVehicleController::GetPlayerHUD() const
{
	return PlayerHUD;
}

void APlayerVehicleController::BeginPlay()
{
	Super::BeginPlay();

	if ((PlayerHUD = CreateWidget<UPlayerHUD>(this, PlayerHUDClass))) {
		PlayerHUD->AddToViewport();
		PlayerHUD->SetPlayerVehicle(PlayerVehicle);
		if (IsValid(PlayerVehicle)){
			PlayerVehicle->OnGearChangedDelegate.AddUObject(PlayerHUD, &UPlayerHUD::UpdateCurrentGear);
			PlayerVehicle->OnGearShiftingDelegate.AddUObject(PlayerHUD, &UPlayerHUD::UpdateIsGearShifting);
		}
	}
}

void APlayerVehicleController::Tick(float Delta)
{
	Super::Tick(Delta);

	if (IsValid(PlayerVehicle) && IsValid(PlayerHUD)) {
		PlayerHUD->UpdateSpeed(PlayerVehicle->GetCurrentTargetSpeed());
	}
}

void APlayerVehicleController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
		Subsystem->AddMappingContext(PlayerInputContext, 0);
	}
}

void APlayerVehicleController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

	PlayerVehicle = CastChecked<ABaseVehicle>(NewPawn);
}
