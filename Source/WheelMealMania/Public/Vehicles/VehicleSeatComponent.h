#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "VehicleSeatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WHEELMEALMANIA_API UVehicleSeatComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UVehicleSeatComponent();

	UFUNCTION(BlueprintCallable, Category="Vehicle|Seats")
	void Sit(AActor* ActorToSit);

	UFUNCTION(BlueprintCallable, Category="Vehicle|Seats")
	void RemoveActorFromSeat();

	UFUNCTION(BlueprintCallable, Category="Vehicle|Seats")
	AActor* GetSittingActor() const;
	
protected:
	virtual void BeginPlay() override;

private:
	TWeakObjectPtr<AActor> OwningActor;
		
};
