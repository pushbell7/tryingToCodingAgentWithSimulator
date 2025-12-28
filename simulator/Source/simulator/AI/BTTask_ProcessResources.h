// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SimulatorTypes.h"
#include "BTTask_ProcessResources.generated.h"

/**
 * Behavior Tree task to process resources at a workshop
 * Converts raw materials into processed goods using recipes
 */
UCLASS()
class SIMULATOR_API UBTTask_ProcessResources : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ProcessResources();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	// Recipe to process
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FCraftingRecipe Recipe;

	// Use craftsman's assigned workshop instead of searching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	bool bUseAssignedWorkshop;

	// Maximum search distance for workshop (if not using assigned)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxSearchDistance;

	// How close to workshop to start processing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float ProcessRadius;

	// Blackboard key for storing target workshop
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName TargetBuildingKey;

private:
	// Track processing end time per component
	TMap<UBehaviorTreeComponent*, float> ProcessEndTimes;
	TMap<UBehaviorTreeComponent*, class ABaseBuilding*> ProcessingWorkshops;
};
