// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NPCCrowdController.h"

#include "Characters/BaseCharacter.h"
#include "Kismet/GameplayStatics.h"

ANPCCrowdController::ANPCCrowdController()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ANPCCrowdController::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickInterval(ControllerTickInterval);
	
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(this, ABaseCharacter::StaticClass(), Actors);
	
	for (AActor* Actor : Actors)
	{
		ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(Actor);
		if (!BaseCharacter) continue;
		Characters.Add(BaseCharacter);
		
		BaseCharacter->SetActorTickEnabled(false);
		BaseCharacter->SetBehaviorTreePaused(true);
		BaseCharacter->SetActorTickInterval(CharacterFarTickInterval);
	}

	PlayerCamera = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;

	UE_LOG(LogTemp, Warning, TEXT("NPC Character Crowd Controller Initialization: \nPlayerCamera: %i, Registered Characters=%i"),
		PlayerCamera != nullptr, Characters.Num());
}

void ANPCCrowdController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetActorTickInterval(ControllerTickInterval);

	for (ABaseCharacter* Character : Characters)
	{
		const float Distance = FVector::Distance(Character->GetActorLocation(), PlayerCamera->GetCameraLocation());

		if (Distance > FarRange && (!IsCharacterVisible(Character) || Distance > FarRange * 2.f))
		{
			Character->SetActorTickEnabled(false);
			Character->SetIsRendering(false);
			Character->SetBehaviorTreePaused(true);
			Character->SetAnimationsEnabled(false);
			continue;
		}
		
		Character->SetActorTickEnabled(true);
		Character->SetIsRendering(true);
		Character->SetBehaviorTreePaused(false);
		Character->SetAnimationsEnabled(true);
		if (Distance > FarRange)
		{
			Character->SetActorTickInterval(CharacterFarTickInterval);
		}
		else
		{
			Character->SetActorTickInterval(CharacterDefaultTickInterval);
		}
	}
}

bool ANPCCrowdController::IsCharacterOutOfFarRange(ABaseCharacter* Character) const
{
	if (!PlayerCamera.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("NPC Character Crowd Controller: Player Camera is invalid"));
		return true;
	}
	return FVector::Distance(Character->GetActorLocation(), PlayerCamera->GetCameraLocation()) > FarRange;
}

bool ANPCCrowdController::IsCharacterVisible(ABaseCharacter* Character) const
{
	if (!PlayerCamera.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("NPC Character Crowd Controller: Player Camera is invalid"));
		return false;
	}

	const FVector CameraForward = PlayerCamera->GetActorForwardVector();
	const FVector DirectionToCharacter = (Character->GetActorLocation() - PlayerCamera->GetCameraLocation()).GetSafeNormal();
	
	if (DirectionToCharacter.Dot(CameraForward) < -0.25)
	{
		return false;
	}

	FHitResult Hit;
	UKismetSystemLibrary::LineTraceSingle(
		this,
		PlayerCamera->GetCameraLocation(),
		Character->GetActorLocation(),
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		{Character},
		EDrawDebugTrace::None,
		Hit,
		true);
	return !Hit.bBlockingHit;
}

