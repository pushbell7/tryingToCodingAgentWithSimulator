// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_WaitRandom.generated.h"

/**
 * Task to wait for a random duration
 */
UCLASS()
class SIMULATOR_API UBTTask_WaitRandom : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_WaitRandom();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	// Minimum wait time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MinWaitTime;

	// Maximum wait time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxWaitTime;

private:
	struct FWaitTaskMemory
	{
		float RemainingTime;
	};

	virtual uint16 GetInstanceMemorySize() const override;
};
