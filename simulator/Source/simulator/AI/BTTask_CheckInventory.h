// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SimulatorTypes.h"
#include "BTTask_CheckInventory.generated.h"

// Inventory check types
UENUM(BlueprintType)
enum EInventoryCheckType
{
	IsFull			UMETA(DisplayName = "Is Full"),
	IsEmpty			UMETA(DisplayName = "Is Empty"),
	HasMinimum		UMETA(DisplayName = "Has Minimum Items"),
	HasResource		UMETA(DisplayName = "Has Specific Resource")
};

/**
 * Behavior Tree task to check inventory status
 * Can check if inventory is full, empty, or has specific resources
 */
UCLASS()
class SIMULATOR_API UBTTask_CheckInventory : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_CheckInventory();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// What to check for
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TEnumAsByte<enum EInventoryCheckType> CheckType;

	// Minimum items required (for CheckType = HasMinimum)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (EditCondition = "CheckType == EInventoryCheckType::HasMinimum"))
	int32 MinimumItems;

	// Specific resource to check (for CheckType = HasResource)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (EditCondition = "CheckType == EInventoryCheckType::HasResource"))
	EResourceType ResourceToCheck;

	// Minimum quantity of resource (for CheckType = HasResource)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (EditCondition = "CheckType == EInventoryCheckType::HasResource"))
	int32 MinimumQuantity;
};
