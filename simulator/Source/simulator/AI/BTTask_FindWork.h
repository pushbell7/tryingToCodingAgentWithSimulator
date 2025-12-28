// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SimulatorTypes.h"
#include "BTTask_FindWork.generated.h"

/**
 * Behavior Tree task to find work assignment
 * Determines what the villager should do based on needs and priorities
 */
UCLASS()
class SIMULATOR_API UBTTask_FindWork : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindWork();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// Priority for gathering work
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float GatheringPriority;

	// Priority for processing work
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float ProcessingPriority;

	// Blackboard key for storing work type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName WorkTypeKey;

	// Blackboard key for storing target zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName TargetZoneKey;

	// Blackboard key for storing target building
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName TargetBuildingKey;
};
