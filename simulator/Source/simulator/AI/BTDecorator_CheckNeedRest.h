// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckNeedRest.generated.h"

/**
 * Decorator to check if villager needs rest
 * Can be based on work cycles, time, or random chance
 */
UCLASS()
class SIMULATOR_API UBTDecorator_CheckNeedRest : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CheckNeedRest();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	// Number of work cycles before needing rest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rest")
	int32 WorkCyclesBeforeRest;

	// Random chance to need rest (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rest")
	float RandomRestChance;

	// Blackboard key to track work cycles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName WorkCycleCountKey;
};
