// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveToBuilding.generated.h"

/**
 * Behavior Tree task to move to a building
 * Uses blackboard to read the target building and moves the AI to it
 */
UCLASS()
class SIMULATOR_API UBTTask_MoveToBuilding : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MoveToBuilding();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// Blackboard key for reading target building
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName TargetBuildingKey;

	// How close to the building we need to get (acceptance radius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AcceptanceRadius;
};
