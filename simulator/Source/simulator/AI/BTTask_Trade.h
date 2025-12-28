// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SimulatorTypes.h"
#include "BTTask_Trade.generated.h"

/**
 * Behavior Tree task for trading resources with merchants
 * Finds merchant and executes trade transaction
 */
UCLASS()
class SIMULATOR_API UBTTask_Trade : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Trade();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// Resource to trade
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	EResourceType ResourceType;

	// Quantity to trade
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	int32 Quantity;

	// Is this a buy transaction? (false = sell)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	bool bBuying;

	// Maximum search distance for market
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxSearchDistance;

	// How close to market to trade
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float TradeRadius;

	// Blackboard key for storing target market
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName TargetBuildingKey;
};
