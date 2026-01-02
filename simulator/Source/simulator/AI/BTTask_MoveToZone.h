// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SimulatorTypes.h"
#include "BTTask_MoveToZone.generated.h"

/**
 * Behavior Tree task to move to a specific zone type location
 * Uses ZoneGrid to find nearest zone of specified type
 */
UCLASS()
class SIMULATOR_API UBTTask_MoveToZone : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MoveToZone();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// Target zone type to move to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	ETerrainZone TargetZoneType;

	// Blackboard key for storing target location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName TargetLocationKey;

	// How close we need to get (acceptance radius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AcceptanceRadius;

	// Maximum search distance for zones
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxSearchDistance;
};
