// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BaseGameMode.generated.h"

class ABaseVehicle;
class UFareTimerComponent;
class ABaseDeliveryTargetArea;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActorToDeliverAdded, AActor*, ActorToDeliver, UFareTimerComponent*, FareTimerComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActorDelivered, AActor*, ActorToDeliver, UFareTimerComponent*, FareTimerComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActorDeliveryFailed, AActor*, ActorToDeliver, UFareTimerComponent*, FareTimerComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBonusTimeAdded, int, BonusTime);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerStartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerStoppedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimerUpdatedSignature, int, Minutes, int, Seconds);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCashAdded, float, Cash, float, EarnedCash);

UCLASS()
class WHEELMEALMANIA_API ABaseGameMode : public AGameModeBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable)
	FOnActorToDeliverAdded OnActorToDeliverAdded;
	UPROPERTY(BlueprintAssignable)
	FOnActorDelivered OnActorDelivered;
	UPROPERTY(BlueprintAssignable)
	FOnActorDelivered OnActorDeliveryFailed;
	
	UPROPERTY(BlueprintAssignable)
	FOnBonusTimeAdded OnBonusTimeAdded;
	
	UPROPERTY(BlueprintAssignable)
	FOnTimerStartedSignature OnTimerStarted;
	UPROPERTY(BlueprintAssignable)
	FOnTimerStoppedSignature OnTimerStopped;
	UPROPERTY(BlueprintAssignable)
	FOnTimerUpdatedSignature OnTimerUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnCashAdded OnCashAdded;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	float CashPerSecond = 10.f;
	
	float Cash = 0.f;
	
	TArray<AActor*> ActorsToDeliver;

public:
	UFUNCTION(BlueprintCallable)
	ABaseVehicle* GetPlayerVehicle() const;
	
	UFUNCTION(BlueprintCallable)
	TArray<AActor*>& GetActorsToDeliver();
	UFUNCTION(BlueprintCallable)
	void AddActorToDeliver(AActor* ActorToDeliver);
	UFUNCTION(BlueprintCallable)
	void RemoveActorToDeliver(AActor* ActorToDeliver);
	UFUNCTION(BlueprintCallable)
	void FailActorDelivery(AActor* ActorToDeliver, UFareTimerComponent* FareTimerComponent);
	
	UFUNCTION(BlueprintImplementableEvent)
	void ActorToDeliverAdded(AActor* ActorToDeliver, UFareTimerComponent* FareTimerComponent);
	UFUNCTION(BlueprintImplementableEvent)
	void ActorDelivered(AActor* DeliveredActor, UFareTimerComponent* FareTimerComponent);
	UFUNCTION(BlueprintImplementableEvent)
	void ActorDeliveryFailed(AActor* ActorToDeliver, UFareTimerComponent* FareTimerComponent);

	UFUNCTION(BlueprintCallable)
	void AddCash(float CashEarned);
	
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
	void TimerTimeAdded(int OldTimeRemaining, int TimeAdded);
	
private:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void DecreaseTime();
	void StopTimer();

private:
	FTimerHandle TimeoutTimerHandle;
	int TimeRemaining = 0;
};
