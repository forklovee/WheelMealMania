// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerVehicleController.generated.h"

class UInputMappingContext;
class UPlayerHUD;
class ABaseVehicle;
/**
 * 
 */
UCLASS()
class WHEELMEALMANIA_API APlayerVehicleController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* PlayerInputContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPlayerHUD> PlayerHUDClass;

	TObjectPtr<ABaseVehicle> PlayerVehicle;
	TObjectPtr<UPlayerHUD> PlayerHUD;

public:
	UFUNCTION(BlueprintCallable, Category = "Input")
	UPlayerHUD* GetPlayerHUD() const;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float Delta) override;
	
	virtual void OnPossess(APawn* Pawn) override;
};
