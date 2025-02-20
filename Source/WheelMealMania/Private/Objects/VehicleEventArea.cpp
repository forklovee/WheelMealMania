// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/VehicleEventArea.h"

#include "Vehicles/PlayerVehicle.h"

// Sets default values
AVehicleEventArea::AVehicleEventArea()
{
	PrimaryActorTick.bCanEverTick = true;
	
	AreaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AreaMesh"));
	SetRootComponent(AreaMesh);

	AreaMesh->SetCollisionProfileName(FName("VehicleTrigger"));
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
	AreaMesh->SetCollisionProfileName(FName("VehicleTrigger"));
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

bool AVehicleEventArea::CanVehicleEnterArea(APlayerVehicle* Vehicle) const
{
	return true;
}

void AVehicleEventArea::PlayerVehicleEntered_Implementation(APlayerVehicle* Vehicle)
{
	PlayerVehicle = Vehicle;
	bIsPlayerVehicleMoving = true;
	SetActorTickEnabled(PlayerVehicle.IsValid());
	if (PlayerVehicle.IsValid())
	{
		UE_LOG(LogTemp, Display, TEXT("Vehicle %s Entered area: %s"), *PlayerVehicle->GetName(), *GetName());
	}
}

void AVehicleEventArea::PlayerVehicleStoppedInside_Implementation(APlayerVehicle* Vehicle)
{
	
}

void AVehicleEventArea::PlayerVehicleExited_Implementation(APlayerVehicle* Vehicle)
{
	VehicleStoppingDeadzoneTimerHandle.Invalidate();
	SetActorTickEnabled(false);
	PlayerVehicle = nullptr;
	bIsPlayerVehicleMoving = false;

	if (IsValid(Vehicle)){
		UE_LOG(LogTemp, Display, TEXT("Vehicle %s Exited area: %s"), *Vehicle->GetName(), *GetName());
	}
}

void AVehicleEventArea::OnVehicleStoppedInsideArea()
{
	if (!PlayerVehicle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Player Vehicle is NULLPTR!!!"));
		return;
	}
	UE_LOG(LogTemp, Display, TEXT("Vehicle %s Stopped inside area: %s"), *PlayerVehicle->GetName(), *GetName());
	
	PlayerVehicleStoppedInside(PlayerVehicle.Get());
}

void AVehicleEventArea::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerVehicle* Vehicle = Cast<APlayerVehicle>(OtherActor);
	
	if (!IsValid(Vehicle)) return;
	if (Vehicle->GetInstigatorController() != GetWorld()->GetFirstPlayerController()) return;
	if (!CanVehicleEnterArea(Vehicle)) return;
	
	PlayerVehicleEntered(Vehicle);
}

void AVehicleEventArea::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != PlayerVehicle.Get()) return;
	
	PlayerVehicleExited(PlayerVehicle.Get());
}


