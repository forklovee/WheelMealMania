#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeliveryTimer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTimerStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTimerStopped, int, TimeRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTimerUpdated, int, TimeRemaining);

UCLASS()
class WHEELMEALMANIA_API UDeliveryTimer : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FTimerStarted OnTimerStarted;
	UPROPERTY(BlueprintAssignable)
	FTimerStopped OnTimerStopped;
	UPROPERTY(BlueprintAssignable)
	FTimerUpdated OnTimerUpdated;
	
private:
	FTimerHandle Timer;
	int TimeRemaining = 0;

public:
	UFUNCTION(BlueprintCallable)
	void StartTimer(const int DeliveryTimeSeconds);
	UFUNCTION(BlueprintCallable)
	void StopTimer();

	UFUNCTION(BlueprintCallable)
	int GetTimeRemaining() const;

private:
	UFUNCTION()
	void UpdateTimer();

};
