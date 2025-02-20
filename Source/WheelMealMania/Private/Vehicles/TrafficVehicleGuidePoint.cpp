// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/TrafficVehicleGuidePoint.h"

// Sets default values
ATrafficVehicleGuidePoint::ATrafficVehicleGuidePoint()
{
	DebugMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugMesh"));
	DebugMesh->SetupAttachment(GetRootComponent());
	DebugMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ATrafficVehicleGuidePoint::BeginPlay()
{
	Super::BeginPlay();
	
}

