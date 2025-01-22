// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/VehicleEventArea.h"

#include "Vehicles/BaseVehicle.h"

// Sets default values
AVehicleEventArea::AVehicleEventArea()
{
	PrimaryActorTick.bCanEverTick = true;
	
	AreaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AreaMesh"));
	SetRootComponent(AreaMesh);

	AreaMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	AreaMesh->SetCanEverAffectNavigation(false);
	AreaMesh->SetAffectDistanceFieldLighting(false);
	AreaMesh->CastShadow = false;
}

void AVehicleEventArea::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	AreaMesh->SetWorldScale3D(AreaScale);
}

void AVehicleEventArea::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(false);
	
	AreaMesh->OnComponentBeginOverlap.AddDynamic(this, &AVehicleEventArea::OnComponentBeginOverlap);
	AreaMesh->OnComponentEndOverlap.AddDynamic(this, &AVehicleEventArea::OnComponentEndOverlap);
}

void AVehicleEventArea::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PlayerVehicle.IsValid())
	{
		SetActorTickEnabled(false);
		return;
	}

	bool bNewIsPlayerVehicleMoving = PlayerVehicle->GetVelocity().Length() > 25.f;
	if (bNewIsPlayerVehicleMoving != bIsPlayerVehicleMoving)
	{
		bIsPlayerVehicleMoving = bNewIsPlayerVehicleMoving;
		if (!bIsPlayerVehicleMoving)
		{
			GetWorldTimerManager().SetTimer(VehicleStoppingDeadzoneTimerHandle,
				this,
				&AVehicleEventArea::OnVehicleStoppedInsideArea,
				0.5);
		}
		else
		{
			VehicleStoppingDeadzoneTimerHandle.Invalidate();
			UE_LOG(LogTemp, Display, TEXT("Vehicle %s Moved after stopping inside area: %s"), *PlayerVehicle->GetName(), *GetName());
		}
	}
}

void AVehicleEventArea::OnVehicleStoppedInsideArea()
{
	PlayerVehicleStoppedInside();
	UE_LOG(LogTemp, Display, TEXT("Vehicle %s Stopped inside area: %s"), *PlayerVehicle->GetName(), *GetName());
}

void AVehicleEventArea::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABaseVehicle* Vehicle = Cast<ABaseVehicle>(OtherActor);
	if (!IsValid(Vehicle)) return;
	if (Vehicle->GetInstigatorController() != GetWorld()->GetFirstPlayerController()) return;
	
	PlayerVehicleEntered();
	PlayerVehicle = Vehicle;
	bIsPlayerVehicleMoving = true;

	SetActorTickEnabled(PlayerVehicle.IsValid());

	UE_LOG(LogTemp, Display, TEXT("Vehicle %s Entered area: %s"), *OtherActor->GetName(), *GetName());
}

void AVehicleEventArea::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != PlayerVehicle.Get()) return;

	PlayerVehicleExited();
	
	SetActorTickEnabled(false);
	PlayerVehicle = nullptr;
	bIsPlayerVehicleMoving = false;
	
	UE_LOG(LogTemp, Display, TEXT("Vehicle %s Exited area: %s"), *OtherActor->GetName(), *GetName());
}


