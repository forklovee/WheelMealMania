// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "NPCVehicle.generated.h"

class ATrafficIntersectionPoint;
class ATrafficVehicleGuidePoint;
class UArrowComponent;
class UBoxComponent;

UCLASS()
class WHEELMEALMANIA_API ANPCVehicle : public APawn
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Components)
	UBoxComponent* CollisionBox;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Components)
	UStaticMeshComponent* Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Components)
	UArrowComponent* ForwardArrow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float Speed = 600.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCVision")
	TEnumAsByte<ECollisionChannel> NPCVisionCollisionChannel = ECollisionChannel::ECC_GameTraceChannel1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCVision")
	float ForwardVisionDistance = 600.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCVision")
	float ForwardVisionSearchDistance = 10000.f;
	
	bool bActivated = false;
	float Throttle = 0.f;
	
private:
	FHitResult GroundHit;
	FTimerHandle IntersectionSelectionTimer;
	
	TWeakObjectPtr<ATrafficIntersectionPoint> CurrentIntersectionPoint;
	bool bIgnoreIntersectionWaitingState = false;
	
	TWeakObjectPtr<ATrafficVehicleGuidePoint> TargetGuidePoint;
	
public:
	// Sets default values for this pawn's properties
	ANPCVehicle();

	void SetIsRendering(bool IsRendering);
	void SetSimulatePhysics(bool bSimulatePhysics);
	
	UFUNCTION(BlueprintCallable)
	void SetThrottle(float NewThrottle);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void HoverAboveGround(float DeltaTime);
	ATrafficVehicleGuidePoint* FindTrafficGuidePoint();
	void DriveToGuidePoint(float DeltaTime);
	bool IsBlockedByObstacle();

	UFUNCTION()
	void SelectIntersectionOpenLane();
	
	UFUNCTION()
	void OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
};

