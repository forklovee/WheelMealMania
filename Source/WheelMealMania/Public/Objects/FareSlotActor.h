// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FareSlotActor.generated.h"

UCLASS()
class WHEELMEALMANIA_API AFareSlotActor : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<AActor> TargetPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SlotRadius = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AFareCharacter> FareCharacterClass;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	USceneComponent* DefaultSpawnPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Component")
	UStaticMeshComponent* AreaMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	UStaticMeshComponent* MarkerMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	class UBillboardComponent* MarkerBillboard;

	AFareCharacter* FareCharacter;

public:	
	// Sets default values for this actor's properties
	AFareSlotActor();

	UFUNCTION(BlueprintCallable)
	bool IsFareCharacterSpawned() const;
	UFUNCTION(BlueprintCallable)
	void SpawnFareCharacter();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	FVector GetFareSpawnPointLocation() const;


};
