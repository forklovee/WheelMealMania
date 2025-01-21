// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BasePickupArea.generated.h"

class ABaseDeliveryTargetArea;

UCLASS()
class ABasePickupArea : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup Area")
	FVector AreaScale = FVector::OneVector;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* AreaMesh;

	TWeakObjectPtr<ABaseDeliveryTargetArea> DeliveryTargetArea;
	
public:	
	ABasePickupArea();

	void SetDeliveryTarget(ABaseDeliveryTargetArea* NewDeliveryTarget);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
	void DeliveryTargetSet(ABaseDeliveryTargetArea* NewDeliveryTarget);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
