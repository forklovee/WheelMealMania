// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NPCCrowdController.generated.h"

class ABaseCharacter;

UCLASS()
class WHEELMEALMANIA_API ANPCCrowdController : public AActor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
	float ControllerTickInterval = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
	float CharacterDefaultTickInterval = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
	float CharacterFarTickInterval = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
	float FarRange = 2500.f;
	
private:
	TArray<ABaseCharacter*> Characters;
	
	TWeakObjectPtr<APlayerCameraManager> PlayerCamera;
	
public:	
	ANPCCrowdController();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	bool IsCharacterOutOfFarRange(ABaseCharacter* Character) const;
	bool IsCharacterVisible(ABaseCharacter* Character) const;
	
};
