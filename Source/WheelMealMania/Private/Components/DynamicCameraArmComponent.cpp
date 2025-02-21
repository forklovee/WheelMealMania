// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/DynamicCameraArmComponent.h"

#include "Kismet/GameplayStatics.h"

UDynamicCameraArmComponent::UDynamicCameraArmComponent()
{
	bInheritPitch = false;
	bInheritRoll = false;
	bInheritYaw = false;
}

void UDynamicCameraArmComponent::SetTargetActor(AActor* NewLookAtTarget)
{
	LookAtTarget = NewLookAtTarget;
}

float UDynamicCameraArmComponent::GetDefaultArmLength() const
{
	return DefaultArmLength;
}

void UDynamicCameraArmComponent::SetDistanceToTarget(float NewDistance)
{
	TargetTargetArmLength = NewDistance;
	UE_LOG(LogTemp, Display, TEXT("SetDistanceToTarget: %f"), TargetArmLength);
}

void UDynamicCameraArmComponent::ResetDistanceToTarget()
{
	TargetTargetArmLength = DefaultArmLength;
}

void UDynamicCameraArmComponent::SetFollowTargetEnabled(bool bNewFollowTargetState)
{
	bFollowTarget = bNewFollowTargetState;
	UE_LOG(LogTemp, Warning, TEXT("Set Follow Target: %i"), bFollowTarget);
}

void UDynamicCameraArmComponent::OverrideTargetForwardVector(FVector TargetForwardVector)
{
	TargetForwardVectorOverride = TargetForwardVector;
}

void UDynamicCameraArmComponent::ClearTargetForwardVectorOverride()
{
	TargetForwardVectorOverride = FVector::ZeroVector;
}

bool UDynamicCameraArmComponent::IsTargetForwardVectorOverriden() const
{
	return FVector::Distance(TargetForwardVectorOverride, FVector::ZeroVector) >= 1.f;
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
	
	UpdateSocketHeight(DeltaTime);
	FollowTargetForwardVector(DeltaTime);
}

void UDynamicCameraArmComponent::UpdateSocketHeight(float DeltaTime)
{
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
	TargetArmLength = FMath::Lerp(TargetArmLength, TargetTargetArmLength, DeltaTime*15.f);
}

void UDynamicCameraArmComponent::FollowTargetForwardVector(float DeltaTime)
{
	if (!LookAtTarget.IsValid())
	{
		LookAtTarget = GetOwner();
	}
	
	const FVector LookAtTargetDirection = IsTargetForwardVectorOverriden() ? TargetForwardVectorOverride : LookAtTarget->GetActorForwardVector();
	FRotator TargetRotation = LookAtTargetDirection.Rotation();
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
}
