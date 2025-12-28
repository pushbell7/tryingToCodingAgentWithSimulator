// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SimulatorTypes.h"
#include "BTTask_WithdrawResources.generated.h"

/**
 * Behavior Tree task to withdraw resources from a storage building
 * Finds storage with desired resources and transfers to inventory
 */
UCLASS()
class SIMULATOR_API UBTTask_WithdrawResources : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_WithdrawResources();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// Resource type to withdraw
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	EResourceType ResourceType;

	// Amount to withdraw
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	int32 WithdrawAmount;

	// Maximum search distance for storage buildings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxSearchDistance;

	// Blackboard key for storing target building
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName TargetBuildingKey;

	// How close to building we need to be to withdraw (acceptance radius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float WithdrawRadius;
};
