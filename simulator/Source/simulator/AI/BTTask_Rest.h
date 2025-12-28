// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Rest.generated.h"

/**
 * Behavior Tree task for villagers to rest at home
 * Finds nearest house and rests for a specified duration
 */
UCLASS()
class SIMULATOR_API UBTTask_Rest : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Rest();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	// Duration to rest in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float RestDuration;

	// Random deviation from rest duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float RandomDeviation;

	// Maximum search distance for house
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxSearchDistance;

	// How close to house to start resting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float RestRadius;

	// Blackboard key for storing target house
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName TargetBuildingKey;

private:
	// Track rest end time per component
	TMap<UBehaviorTreeComponent*, float> RestEndTimes;
};
