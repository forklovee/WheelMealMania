// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FareTimerComponent.generated.h"

class ABaseDeliveryTargetArea;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerStarted, int, TimeRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerStopped, int, TimeRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerUpdated, int, TimeRemaining);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WHEELMEALMANIA_API UFareTimerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnTimerStarted OnTimerStarted;
	UPROPERTY(BlueprintAssignable)
	FOnTimerStopped OnTimerStopped;
	UPROPERTY(BlueprintAssignable)
	FOnTimerUpdated OnTimerUpdated;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Timer", meta=(AllowPrivateAccess=true))
	int MinDeliveryTime = 5;
	
private:
	FTimerHandle TimerHandle;
	int MaxDeliveryTime = 0;
	int TimeRemaining = 0;

	TWeakObjectPtr<AActor> DeliveryTarget;
	
public:
	UFareTimerComponent();
	
	UFUNCTION(BlueprintCallable)
	void StartTimer(const int TimeSeconds, AActor* NewDeliveryTarget);
	UFUNCTION(BlueprintCallable)
	void StopTimer();

	UFUNCTION(BlueprintCallable)
	int GetTimeRemaining() const;

	UFUNCTION(BlueprintCallable)
	int GetMaxDeliveryTime() const;
	
	UFUNCTION(BlueprintCallable)
	int GetMinDeliveryTime() const;

	UFUNCTION(BlueprintCallable)
	AActor* GetDeliveryTarget() const;
	
protected:
	virtual void BeginPlay() override;

private:
	void UpdateTimer();
		
};
