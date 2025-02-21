
#include "Characters/BaseCharacter.h"


ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABaseCharacter::SetIsRendering(bool isRendering)
{
	GetMesh()->SetVisibility(isRendering);
}

void ABaseCharacter::SetAnimationsEnabled(bool bAnimationsEnabled) const
{
	GetMesh()->bPauseAnims = bAnimationsEnabled;
	GetMesh()->SetComponentTickInterval((bAnimationsEnabled) ? 0.f : 5.f);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

