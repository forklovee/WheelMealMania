// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MainGameInstance.generated.h"


USTRUCT(BlueprintType)
struct FPlayerData
{
	GENERATED_BODY()

	FPlayerData()
	{
		GameplayTimerTime = -1;
	}

	UPROPERTY(BlueprintReadWrite)
	int GameplayTimerTime = -1;
};

UCLASS()
class WHEELMEALMANIA_API UMainGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FPlayerData PlayerData;

protected:
	virtual void Init() override; 
};
