// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IntersectionController.generated.h"

class ATrafficIntersectionPoint;

USTRUCT(BlueprintType)
struct FIntersectionPointState
{
	GENERATED_BODY()
	FIntersectionPointState()
	{
		IntersectionPoint = nullptr;
		bIsWaiting = true;
	}
	FIntersectionPointState(ATrafficIntersectionPoint* NewIntersectionPoint, bool bNewIsWaiting = true)
	{
		IntersectionPoint = NewIntersectionPoint;
		bIsWaiting = bNewIsWaiting;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<ATrafficIntersectionPoint> IntersectionPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWaiting = true;
};

USTRUCT(BlueprintType)
struct FIntersectionSequence
{
	GENERATED_BODY()
	FIntersectionSequence()
	{
		States = {};
		Duration = 10;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FIntersectionPointState> States;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Duration = 10;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSequenceStartedSignature, AIntersectionController*, IntersectionController, FIntersectionSequence, IntersectionSequence);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSequenceUpdateSignature, AIntersectionController*, IntersectionController, int, TimeLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSequencesEndedSignature, AIntersectionController*, IntersectionController);

UCLASS()
class WHEELMEALMANIA_API AIntersectionController : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSequenceStartedSignature SequenceStarted;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSequenceUpdateSignature SequenceTimeTick;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSequencesEndedSignature SequencesEnded;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoStart = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLoop = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FIntersectionSequence> Sequences;

private:
	FTimerHandle SequenceTimer;
	FTimerHandle TickTimer;
	uint8 SequenceIndex = -1;

	TArray<ATrafficIntersectionPoint*> IntersectionPoints;
	
public:
	AIntersectionController();

	UFUNCTION(BlueprintCallable)
	void Start();
	UFUNCTION(BlueprintCallable)
	uint8 GetSequenceIndex() const;
	UFUNCTION(BlueprintImplementableEvent)
	void OnSequenceTimeTick(FIntersectionSequence IntersectionSequence, int TimeLeft);

	UFUNCTION(BlueprintCallable)
	TArray<ATrafficIntersectionPoint*> GetOpenIntersectionPointsOfLane(bool bRightLane);
	
protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void StartNewSequence();
	
	UFUNCTION()
	void SequenceTick();

	void Stop();

	void InitializeIntersectionPoints();
	
};
