// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BaseGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerStartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerStoppedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimerUpdatedSignature, int, Minutes, int, Seconds);

UCLASS()
class WHEELMEALMANIA_API ABaseGameMode : public AGameModeBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable)
	FOnTimerStartedSignature OnTimerStarted;
	UPROPERTY(BlueprintAssignable)
	FOnTimerStoppedSignature OnTimerStopped;
	UPROPERTY(BlueprintAssignable)
	FOnTimerUpdatedSignature OnTimerUpdated;

public:
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
	UFUNCTION()
	void DecreaseTime();
	void StopTimer();

private:
	FTimerHandle TimeoutTimerHandle;
	int TimeRemaining = 0;
};
