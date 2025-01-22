// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VehicleEventArea.generated.h"

UCLASS()
class WHEELMEALMANIA_API AVehicleEventArea : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup Area")
	FVector AreaScale = FVector::OneVector;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* AreaMesh;

private:
	TWeakObjectPtr<class ABaseVehicle> PlayerVehicle;
	bool bIsPlayerVehicleMoving = false;

	FTimerHandle VehicleStoppingDeadzoneTimerHandle;
	
public:	
	AVehicleEventArea();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category="Pickup Area")
	void PlayerVehicleEntered();

	UFUNCTION(BlueprintImplementableEvent, Category="Pickup Area")
	void PlayerVehicleStoppedInside();
	
	UFUNCTION(BlueprintImplementableEvent, Category="Pickup Area")
	void PlayerVehicleExited();
	
	virtual void OnVehicleStoppedInsideArea();
	
	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
