// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VehicleEventArea.generated.h"

class APlayerVehicle;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class APlayerVehicle> PlayerVehicle;
private:
	bool bIsPlayerVehicleMoving = false;

	FTimerHandle VehicleStoppingDeadzoneTimerHandle;
	
public:	
	AVehicleEventArea();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual bool CanVehicleEnterArea(APlayerVehicle* Vehicle) const;
	
	UFUNCTION(BlueprintNativeEvent, Category="Pickup Area")
	void PlayerVehicleEntered(APlayerVehicle* Vehicle);
	virtual void PlayerVehicleEntered_Implementation(APlayerVehicle* Vehicle);

	UFUNCTION(BlueprintNativeEvent, Category="Pickup Area")
	void PlayerVehicleStoppedInside(APlayerVehicle* Vehicle);
	virtual void PlayerVehicleStoppedInside_Implementation(APlayerVehicle* Vehicle);
	
	UFUNCTION(BlueprintNativeEvent, Category="Pickup Area")
	void PlayerVehicleExited(APlayerVehicle* Vehicle);
	virtual void PlayerVehicleExited_Implementation(APlayerVehicle* Vehicle);

	UFUNCTION()
	virtual void OnVehicleStoppedInsideArea();
	
	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
