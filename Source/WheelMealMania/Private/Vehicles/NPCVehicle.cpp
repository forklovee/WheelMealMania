// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/NPCVehicle.h"

#include "Landscape.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Vehicles/IntersectionController.h"
#include "Vehicles/TrafficIntersectionPoint.h"
#include "Vehicles/TrafficVehicleGuidePoint.h"

ANPCVehicle::ANPCVehicle()
{
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(GetRootComponent());
	CollisionBox->SetLinearDamping(10.f);
	CollisionBox->SetAngularDamping(20.f);
	CollisionBox->SetEnableGravity(true);
	CollisionBox->SetSimulatePhysics(true);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(CollisionBox);

	ForwardArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardArrow"));
	ForwardArrow->SetupAttachment(CollisionBox);
	
	PrimaryActorTick.bCanEverTick = true;
}

void ANPCVehicle::SetThrottle(float NewThrottle)
{
	Throttle = NewThrottle;
}

void ANPCVehicle::BeginPlay()
{
	Super::BeginPlay();
	CollisionBox->OnComponentHit.AddDynamic(this, &ANPCVehicle::OnComponentHit);

	TargetGuidePoint = FindTrafficGuidePoint();
}

void ANPCVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HoverAboveGround(DeltaTime);

	if (IsBlockedByObstacle())
	{
		SetThrottle(0.f);
	}
	else
	{
		DriveToGuidePoint(DeltaTime);
	}

	SetActorLocation(
		GetActorLocation() + Throttle * CollisionBox->GetForwardVector() * Speed * DeltaTime,
		true);
}

void ANPCVehicle::SetIsRendering(bool IsRendering)
{
	Mesh->SetVisibility(IsRendering);
}

void ANPCVehicle::SetSimulatePhysics(bool bSimulatePhysics)
{
	CollisionBox->SetSimulatePhysics(bSimulatePhysics);
}

void ANPCVehicle::HoverAboveGround(float DeltaTime)
{
	const FVector VehicleLocation = GetActorLocation();
	UKismetSystemLibrary::LineTraceSingle(
		this,
		VehicleLocation,
		VehicleLocation + FVector::DownVector * 1000.f,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		{},
		EDrawDebugTrace::None,
		GroundHit,
		true,
		FLinearColor::Red);

	if (!GroundHit.bBlockingHit)
	{
		return;
	}
	
	SetActorLocation(
		FMath::VInterpTo(VehicleLocation, GroundHit.Location + FVector::UpVector * 65.0, DeltaTime, 10.f),
		true
	);

	SetActorRotation(
		FMath::RInterpTo(GetActorRotation(),
			(-GroundHit.ImpactNormal.Cross(CollisionBox->GetRightVector())).ToOrientationRotator(),
			DeltaTime,
			10.f)
	);
}

ATrafficVehicleGuidePoint* ANPCVehicle::FindTrafficGuidePoint()
{
	const FVector VehicleLocation = GetActorLocation();
	const FVector ForwardDirection = GetActorForwardVector();
	
	TArray<FHitResult> ForwardCasts;
	UKismetSystemLibrary::SphereTraceMultiForObjects(
		this,
		VehicleLocation,
		VehicleLocation + ForwardDirection * ForwardVisionSearchDistance,
		2000.f,
		{EObjectTypeQuery::ObjectTypeQuery7},
		false,
		{TargetGuidePoint.Get()},
		EDrawDebugTrace::None,
		ForwardCasts,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		25.f);

	ATrafficVehicleGuidePoint* TrafficGuidePoint = nullptr;
	float DistanceToLastTrafficGuidePoint = 1000000.f;
	for (FHitResult& ForwardCast : ForwardCasts)
	{
		ATrafficVehicleGuidePoint* NewTrafficGuidePoint = Cast<ATrafficVehicleGuidePoint>(ForwardCast.GetActor());
		if (!NewTrafficGuidePoint) continue;
		
		const FVector DirectionToPoint = (VehicleLocation - NewTrafficGuidePoint->GetActorLocation()).GetSafeNormal();
		const float Dot = -FVector::DotProduct(DirectionToPoint, ForwardDirection);
		
		const float DistanceToPoint = FVector::Dist(VehicleLocation, NewTrafficGuidePoint->GetActorLocation());
		if (Dot > 0.4f && DistanceToPoint < DistanceToLastTrafficGuidePoint &&
			(!TargetGuidePoint.IsValid() ||
			(TargetGuidePoint.IsValid() &&
				((TargetGuidePoint->ActorHasTag("RightLane") && NewTrafficGuidePoint->ActorHasTag("RightLane")) ||
				(TargetGuidePoint->ActorHasTag("LeftLane") && NewTrafficGuidePoint->ActorHasTag("LeftLane"))))
				))
		{
			DistanceToLastTrafficGuidePoint = DistanceToPoint;
			TrafficGuidePoint = NewTrafficGuidePoint;
		}
	}
	if (TrafficGuidePoint)
	{
		CurrentIntersectionPoint = Cast<ATrafficIntersectionPoint>(TrafficGuidePoint);
		bIgnoreIntersectionWaitingState = false;

		// UE_LOG(LogTemp, Warning, TEXT("Guide Point Set: %s [is intersection = %i]"), *TrafficGuidePoint->GetName(), CurrentIntersectionPoint != nullptr);
	}
	return TrafficGuidePoint;
}

void ANPCVehicle::DriveToGuidePoint(float DeltaTime)
{
	if (!TargetGuidePoint.IsValid())
	{
		return;
	}
	
	const float DistanceToTarget = FVector::Distance(GetActorLocation(), TargetGuidePoint->GetActorLocation());
	if (DistanceToTarget > 200.f)
	{
		SetThrottle(1.f);

		const FVector FlattenedDirectionToPoint = ((TargetGuidePoint->GetActorLocation() - GetActorLocation()) * FVector(1, 1, 0)).GetSafeNormal();
		const float Yaw = FlattenedDirectionToPoint.ToOrientationRotator().Yaw;
		const FRotator TargetRotation = FRotator(GetActorRotation().Pitch, Yaw, GetActorRotation().Roll);
		SetActorRotation(
			FMath::RInterpTo(
				GetActorRotation(),
				TargetRotation,
				DeltaTime,
				5.f)
		);
		return;
	}

	// Arrived at target.
	
	SetThrottle(0.f);
	if (CurrentIntersectionPoint.IsValid())
	{
		// Has to wait.
		if (CurrentIntersectionPoint->GetWaitingState())
		{
			return;
		}

		if (!IntersectionSelectionTimer.IsValid())
		{
			GetWorldTimerManager().SetTimer(IntersectionSelectionTimer, this, &ANPCVehicle::SelectIntersectionOpenLane, .45f, false);
		}
		return;
	}
	
	TargetGuidePoint = FindTrafficGuidePoint();
}

bool ANPCVehicle::IsBlockedByObstacle()
{
	FHitResult Hit;
	UKismetSystemLibrary::SphereTraceSingle(
		this,
		GetActorLocation(),
		GetActorLocation() + CollisionBox->GetForwardVector() * ForwardVisionDistance,
		200.f,
		UEngineTypes::ConvertToTraceType(NPCVisionCollisionChannel),
		false,
		{this},
		EDrawDebugTrace::None,
		Hit,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		5.f);
	
	if (IsValid(Hit.GetActor()))
	{
		// UE_LOG(LogTemp, Warning, TEXT("%s: Blocked by %s"), *GetName(), *Hit.GetActor()->GetName());
	}
	return Hit.bBlockingHit;
}

void ANPCVehicle::SelectIntersectionOpenLane()
{
	GetWorldTimerManager().ClearTimer(IntersectionSelectionTimer);
	
	AIntersectionController* IntersectionController = CurrentIntersectionPoint->GetIntersectionController();
	if (!IntersectionController)
	{
		UE_LOG(LogTemp, Error, TEXT("%s Intersection Controller not found"), *CurrentIntersectionPoint->GetName());
		return;
	}
		
	const bool TargetIsRightLane = CurrentIntersectionPoint->ActorHasTag("RightLane");
		
	// Get Random Open lane and set it as a guide point
	TArray<ATrafficIntersectionPoint*> OpenLanes = IntersectionController->GetOpenIntersectionPointsOfLane(TargetIsRightLane);
	OpenLanes.Remove(CurrentIntersectionPoint.Get());

	if (OpenLanes.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("%s No open lanes."), *CurrentIntersectionPoint->GetName());
		return;
	}
	
	TargetGuidePoint = OpenLanes[FMath::RandRange(0, OpenLanes.Num() - 1)];
	CurrentIntersectionPoint = nullptr;
}

void ANPCVehicle::OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor != UGameplayStatics::GetPlayerPawn(this, 0))
	{
		return;
	}
	bActivated = false;
}
