
#include "Objects/BasePickupArea.h"
#include "Objects/BaseDeliveryTargetArea.h"

// Sets default values
ABasePickupArea::ABasePickupArea()
{
	PrimaryActorTick.bCanEverTick = true;

	AreaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AreaMesh"));
	SetRootComponent(AreaMesh);

	AreaMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	AreaMesh->SetCanEverAffectNavigation(false);
	AreaMesh->SetAffectDistanceFieldLighting(false);
	AreaMesh->CastShadow = false;
}

void ABasePickupArea::SetDeliveryTarget(ABaseDeliveryTargetArea* InDeliveryTarget)
{
	DeliveryTargetArea = InDeliveryTarget;
	DeliveryTargetSet(DeliveryTargetArea.Get());
}

void ABasePickupArea::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	AreaMesh->SetWorldScale3D(AreaScale);
}

void ABasePickupArea::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABasePickupArea::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

