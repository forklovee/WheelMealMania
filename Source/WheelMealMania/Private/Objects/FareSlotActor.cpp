// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/FareSlotActor.h"

#include "Components/BillboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Characters/FareCharacter.h"

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

	FVector LocationSpawn = GetFareSpawnPointLocation();
	if (FareCharacter = GetWorld()->SpawnActor<AFareCharacter>(
		FareCharacterClass.Get(), LocationSpawn, FRotator::ZeroRotator)) {
		UE_LOG(LogTemp, Display, TEXT("Spawned Fare Character!"))
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn Fare Character!"))
	}
}

FVector AFareSlotActor::GetFareSpawnPointLocation() const
{
	FHitResult LineTraceHitResult;
	UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		GetActorLocation() + FVector(0.f, 0.f, SlotRadius*.25f),
		GetActorLocation() - FVector(0.f, 0.f, SlotRadius),
		75.f,
		220.f,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		{},
		EDrawDebugTrace::ForDuration,
		LineTraceHitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		60.f
	);

	if (!LineTraceHitResult.bBlockingHit) {
		return GetActorLocation();
	}
	return LineTraceHitResult.Location;
}

