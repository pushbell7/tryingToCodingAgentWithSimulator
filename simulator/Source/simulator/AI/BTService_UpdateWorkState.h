// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateWorkState.generated.h"

/**
 * Service to update work-related blackboard values
 * Tracks work cycles, updates state, etc.
 */
UCLASS()
class SIMULATOR_API UBTService_UpdateWorkState : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateWorkState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// Blackboard key for work cycle count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName WorkCycleCountKey;

	// Blackboard key for last work result
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName LastWorkResultKey;

	// Should increment work cycle when task succeeds?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Service")
	bool bIncrementOnSuccess;
};
