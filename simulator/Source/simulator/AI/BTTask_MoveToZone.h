// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SimulatorTypes.h"
#include "BTTask_MoveToZone.generated.h"

/**
 * Behavior Tree task to move to a terrain zone
 * Uses blackboard to read the target zone and moves the AI to it
 */
UCLASS()
class SIMULATOR_API UBTTask_MoveToZone : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MoveToZone();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// Blackboard key for reading target zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName TargetZoneKey;

	// How close to the zone center we need to get (acceptance radius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AcceptanceRadius;

	// Whether to move to exact center or just enter the zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	bool bMoveToCenter;
};
