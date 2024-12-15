// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/BillboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Characters/FareCharacter.h"

#include "Objects/FareSlotActor.h"

// Sets default values
AFareSlotActor::AFareSlotActor()
{
	DefaultSpawnPoint = CreateDefaultSubobject<USceneComponent>(FName("DefaultSpawnPoint"));
	RootComponent = DefaultSpawnPoint;

	AreaMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("AreaMesh"));
	AreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AreaMesh->SetCanEverAffectNavigation(false);
	AreaMesh->SetAffectDistanceFieldLighting(false);
	AreaMesh->SetupAttachment(RootComponent);

	MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("MarkerMesh"));
	MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MarkerMesh->SetCanEverAffectNavigation(false);
	MarkerMesh->SetAffectDistanceFieldLighting(false);
	MarkerMesh->SetupAttachment(RootComponent);

	MarkerBillboard = CreateDefaultSubobject<UBillboardComponent>(FName("MarkerBillboard"));
	MarkerBillboard->SetupAttachment(MarkerMesh);

	PrimaryActorTick.bCanEverTick = false;
}

void AFareSlotActor::OnConstruction(const FTransform& Transform)
{
	if (AreaMesh) {
		AreaMesh->SetWorldScale3D(FVector(0.f, 0.f, 2.f) + FVector(1.f, 1.f, 0.0f) * SlotRadius * 0.01f * 2.f);
	}

}

// Called when the game starts or when spawned
void AFareSlotActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (MarkerMesh) {
		MarkerMesh->SetWorldLocation(GetFareSpawnPointLocation() + FVector::UpVector*300.f);
	}

	SpawnFareCharacter();
}

bool AFareSlotActor::IsFareCharacterSpawned() const
{
	return IsValid(FareCharacter);
}

void AFareSlotActor::SpawnFareCharacter()
{
	if (IsFareCharacterSpawned()) {
		return;
	}

	if (!IsValid(FareCharacterClass)) {
		UE_LOG(LogTemp, Error, TEXT("Fare Character Class not set!"))
		return;
	}

	if (FareCharacter = GetWorld()->SpawnActor<AFareCharacter>(FareCharacterClass.Get())) {
		FareCharacter->SetActorLocation(
			GetFareSpawnPointLocation()
		);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn Fare Character!"))
	}
}

FVector AFareSlotActor::GetFareSpawnPointLocation() const
{
	FHitResult LineTraceHitResult;
	UKismetSystemLibrary::LineTraceSingle(
		this,
		GetActorLocation() + FVector(0.f, SlotRadius, 0.f),
		GetActorLocation() - FVector(0.f, SlotRadius, 0.f),
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		{},
		EDrawDebugTrace::ForOneFrame,
		LineTraceHitResult,
		true
	);

	return LineTraceHitResult.bBlockingHit ? LineTraceHitResult.ImpactPoint : GetActorLocation();
}

