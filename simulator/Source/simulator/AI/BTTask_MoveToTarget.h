// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveToTarget.generated.h"

/**
 * Move to a target location (Zone, Building, or Vector)
 * Custom movement task for villager AI
 */
UCLASS()
class SIMULATOR_API UBTTask_MoveToTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MoveToTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

protected:
	// Blackboard key for target (can be TargetZone, TargetBuilding, or Vector)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetKey;

	// Acceptable radius to target
	UPROPERTY(EditAnywhere, Category = "Movement")
	float AcceptanceRadius;

	// Should we run or walk?
	UPROPERTY(EditAnywhere, Category = "Movement")
	bool bShouldRun;

	// Get target location from blackboard
	bool GetTargetLocation(UBehaviorTreeComponent& OwnerComp, FVector& OutLocation);
};
