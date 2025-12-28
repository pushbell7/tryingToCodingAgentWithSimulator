// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SimulatorTypes.h"
#include "BTTask_GatherResource.generated.h"

/**
 * Behavior Tree task to gather resources from a terrain zone
 * Requires the AI to have an InventoryComponent
 */
UCLASS()
class SIMULATOR_API UBTTask_GatherResource : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_GatherResource();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// Target zone type to gather from
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	ETerrainZone TargetZoneType;

	// Amount to gather per work cycle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	int32 GatherAmount;

	// Maximum search distance for zones
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxSearchDistance;

	// Blackboard key for storing target zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName TargetZoneKey;
};
