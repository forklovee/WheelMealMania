// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerVehicleController.h"
#include "UI/PlayerHUD.h"
#include "Vehicles/BaseVehicle.h"
#include "EnhancedInputSubsystems.h"

void APlayerVehicleController::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerHUD = CreateWidget<UPlayerHUD>(this, PlayerHUDClass)) {
		PlayerHUD->AddToViewport();
	}
}

void APlayerVehicleController::Tick(float Delta)
{
	Super::Tick(Delta);

	if (IsValid(PlayerVehicle) && IsValid(PlayerHUD)) {
		PlayerHUD->UpdateSpeed(PlayerVehicle->GetVelocity().Length());
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
