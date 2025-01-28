// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputBufferComponent.generated.h"

USTRUCT(BlueprintType)
struct FComboKeys
{
	GENERATED_BODY()

	FComboKeys() {}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<class UInputAction*> Keys;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WHEELMEALMANIA_API UInputBufferComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	TMap<FName, FComboKeys> Combos;
	
private:
	TArray<FString> Buffer;

public:	
	UInputBufferComponent();
	
	UFUNCTION(BlueprintCallable)
	void PushInput(FString InputName);

protected:
	virtual void BeginPlay() override;

		
};
