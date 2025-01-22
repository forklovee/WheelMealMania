// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BaseGameMode.generated.h"

class ABaseDeliveryTargetArea;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewDeliveryTargetAdded, ABaseDeliveryTargetArea*, DeliveryTargetArea);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerStartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerStoppedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimerUpdatedSignature, int, Minutes, int, Seconds);

UCLASS()
class WHEELMEALMANIA_API ABaseGameMode : public AGameModeBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable)
	FOnNewDeliveryTargetAdded OnNewDeliveryTargetAdded;
	
	UPROPERTY(BlueprintAssignable)
	FOnTimerStartedSignature OnTimerStarted;
	UPROPERTY(BlueprintAssignable)
	FOnTimerStoppedSignature OnTimerStopped;
	UPROPERTY(BlueprintAssignable)
	FOnTimerUpdatedSignature OnTimerUpdated;

protected:
	TWeakObjectPtr<class ABaseVehicle> PlayerVehicle;
	
	TArray<ABaseDeliveryTargetArea*> DeliveryTargets;

public:
	UFUNCTION(BlueprintCallable)
	ABaseVehicle* GetPlayerVehicle();
	
	UFUNCTION(BlueprintCallable)
	TArray<ABaseDeliveryTargetArea*>& GetDeliveryTargets();
	UFUNCTION(BlueprintCallable)
	void AddDeliveryTarget(ABaseDeliveryTargetArea* NewDeliveryTargetArea);

	UFUNCTION(BlueprintImplementableEvent)
	void DeliveryTargetAdded(ABaseDeliveryTargetArea* NewDeliveryTargetArea);
	
	UFUNCTION(BlueprintCallable)
	void StartTimer(int TimeSeconds);
	UFUNCTION(BlueprintCallable)
	void AddTime(int TimeSeconds);
	UFUNCTION(BlueprintCallable)
	int GetRemainingTimeSeconds() const;
	UFUNCTION(BlueprintCallable)
	int GetRemainingTimeMinutes() const;

	UFUNCTION(BlueprintImplementableEvent)
	void TimerStarted(int TimeSeconds);
	UFUNCTION(BlueprintImplementableEvent)
	void TimerStopped();
	UFUNCTION(BlueprintImplementableEvent)
	void TimerUpdated(int CurrentTimeRemaining);
	UFUNCTION(BlueprintImplementableEvent)
	void TimerTimeAdded(int TimeAdded);
	
private:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void DecreaseTime();
	void StopTimer();

private:
	FTimerHandle TimeoutTimerHandle;
	int TimeRemaining = 0;
};
