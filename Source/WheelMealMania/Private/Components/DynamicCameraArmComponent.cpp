// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/DynamicCameraArmComponent.h"

#include "Kismet/GameplayStatics.h"

UDynamicCameraArmComponent::UDynamicCameraArmComponent()
{
	bInheritPitch = false;
	bInheritRoll = false;
	bInheritYaw = false;
}

float UDynamicCameraArmComponent::GetDefaultArmLength() const
{
	return DefaultArmLength;
}

void UDynamicCameraArmComponent::SetFollowTargetEnabled_Implementation(bool bNewFollowTargetState)
{
	bFollowTarget = bNewFollowTargetState;
	UE_LOG(LogTemp, Warning, TEXT("Set Follow Target: %i"), bFollowTarget);
}

void UDynamicCameraArmComponent::ResetDistanceToTarget_Implementation()
{
	TargetTargetArmLength = DefaultArmLength;
}

void UDynamicCameraArmComponent::SetDistanceToTarget_Implementation(float NewDistance)
{
	TargetTargetArmLength = NewDistance;
	UE_LOG(LogTemp, Display, TEXT("SetDistanceToTarget: %f"), TargetArmLength);
}

void UDynamicCameraArmComponent::SetCameraLookAtTarget_Implementation(AActor* NewLookAtTarget)
{
	LookAtTarget = NewLookAtTarget;
}

void UDynamicCameraArmComponent::BeginPlay()
{
	Super::BeginPlay();

	DefaultArmLength = TargetArmLength;
	TargetTargetArmLength = DefaultArmLength;
	
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
		EDrawDebugTrace::None,
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

	FVector DirectionToLookAtTarget = -LastDirectionToLookAtTarget;
	if (!LookAtTarget.IsValid())
	{
		LookAtTarget = GetOwner();
	}

	if (bFollowTarget && LookAtTarget == GetOwner())
	{
		DirectionToLookAtTarget = GetOwner()->GetActorForwardVector();
		LastDirectionToLookAtTarget = DirectionToLookAtTarget;
	}
	else
	{
		DirectionToLookAtTarget = LastDirectionToLookAtTarget;
	}
	
	FRotator TargetRotation = DirectionToLookAtTarget.Rotation();
	if (!bAffectYaw)
	{
		TargetRotation.Yaw = 0.f;
	}
	if (!bAffectPitch)
	{
		TargetRotation.Pitch = 0.f;
	}
	if (!bAffectRoll)
	{
		TargetRotation.Roll = 0.f;
	}
	
	SetRelativeRotation(
		FMath::RInterpTo(GetRelativeRotation(), TargetRotation, DeltaTime, 15.f)
	);

	TargetArmLength = FMath::Lerp(TargetArmLength, TargetTargetArmLength, DeltaTime*15.f);
}
