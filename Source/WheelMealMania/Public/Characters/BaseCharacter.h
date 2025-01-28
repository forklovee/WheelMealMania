#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class UBehaviorTree;

UCLASS()
class WHEELMEALMANIA_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseCharacter();

protected:
	UPROPERTY(BlueprintReadWrite)
	FVector SpawnLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UBehaviorTree> DefaultBehaviorTree;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	
	
};
