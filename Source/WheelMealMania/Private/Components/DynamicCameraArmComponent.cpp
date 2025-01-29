// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/DynamicCameraArmComponent.h"

#include "HeadMountedDisplayTypes.h"
#include "Kismet/GameplayStatics.h"

void UDynamicCameraArmComponent::SetDistanceToTarget_Implementation(float NewDistance)
{
	TargetArmLength = NewDistance;
}

void UDynamicCameraArmComponent::SetCameraLookAtTarget_Implementation(AActor* NewLookAtTarget)
{
	LookAtTarget = NewLookAtTarget;
}

void UDynamicCameraArmComponent::BeginPlay()
{
	Super::BeginPlay();

	DefaultSocketOffset = SocketOffset;
}

void UDynamicCameraArmComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	const FVector SocketLocation = GetSocketLocation(SocketName);
	
	FHitResult SpringArmHit;
	UKismetSystemLibrary::SphereTraceSingle(
		this,
		GetComponentLocation(),
		SocketLocation,
		ProbeSize,
		UEngineTypes::ConvertToTraceType(ProbeChannel),
		false,
		{ GetOwner() },
		EDrawDebugTrace::None,
		SpringArmHit,
		true,
		FLinearColor::Blue,
		FLinearColor::Red,
		0.1f
	);
	
	const float HeightHitLength = 5.f*DefaultSocketOffset.Z;
	
	FHitResult HeightHit;
	UKismetSystemLibrary::SphereTraceSingle(
		this,
		SocketLocation,
		SocketLocation + FVector::DownVector*HeightHitLength,
		ProbeSize * 1.5f,
		UEngineTypes::ConvertToTraceType(ProbeChannel),
		false,
		{ GetOwner() },
		EDrawDebugTrace::ForDuration,
		HeightHit,
		true,
		FLinearColor::Blue,
		FLinearColor::Red,
		0.1f
	);
	
	float CameraHeight = DefaultSocketOffset.Z;
	if (HeightHit.bBlockingHit)
	{
		const float HeightHitOffset = FVector::Distance(SocketLocation, HeightHit.ImpactPoint);
		if (HeightHitOffset < DefaultSocketOffset.Z)
		{
			CameraHeight += 2.5f*(DefaultSocketOffset.Z - HeightHitOffset);
		}
	}
	
	SocketOffset.Z = FMath::Lerp(SocketOffset.Z, CameraHeight, DeltaTime*15.f);
}
