// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_DepositResources.generated.h"

/**
 * Behavior Tree task to deposit resources into a storage building
 * Finds nearest available storage and transfers inventory contents
 */
UCLASS()
class SIMULATOR_API UBTTask_DepositResources : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_DepositResources();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// Maximum search distance for storage buildings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxSearchDistance;

	// Blackboard key for storing target building
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName TargetBuildingKey;

	// How close to building we need to be to deposit (acceptance radius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float DepositRadius;
};
