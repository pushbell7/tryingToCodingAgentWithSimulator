// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "VillagerAIController.generated.h"

UCLASS()
class SIMULATOR_API AVillagerAIController : public AAIController
{
	GENERATED_BODY()

public:
	AVillagerAIController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

public:
	// Behavior Tree asset
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	class UBehaviorTree* BehaviorTreeAsset;

	// Blackboard asset
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	class UBlackboardData* BlackboardAsset;

protected:
	UPROPERTY()
	class ABaseVillager* ControlledVillager;
};
