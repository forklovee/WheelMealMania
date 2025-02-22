// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerVehicleController.generated.h"

class APlayerVehicle;
class UInputMappingContext;
class UPlayerHUD;
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
	
	bool bIsPauseMenuOpen = false;
	bool bCanTogglePauseMenu = true;
	
	TObjectPtr<APlayerVehicle> PlayerVehicle;
	TObjectPtr<UPlayerHUD> PlayerHUD;

public:
	void TogglePauseMenuInput(const struct FInputActionValue& InputValue);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input")
	void TogglePause(bool bPauseMenuState);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Input")
	void OnPauseToggled(bool bPauseMenuOpened);
	
	UFUNCTION(BlueprintCallable, Category = "Input")
	UPlayerHUD* GetPlayerHUD() const;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float Delta) override;
	
	virtual void OnPossess(APawn* Pawn) override;
};
