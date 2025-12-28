// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTDecorator_CheckNeedRest.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_CheckNeedRest::UBTDecorator_CheckNeedRest()
{
	NodeName = "Check Need Rest";
	WorkCyclesBeforeRest = 3; // Rest after 3 work cycles
	RandomRestChance = 0.1f;   // 10% random chance each check
	WorkCycleCountKey = FName("WorkCycleCount");
}

bool UBTDecorator_CheckNeedRest::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return false;
	}

	// Get current work cycle count
	int32 CurrentCycles = BlackboardComp->GetValueAsInt(WorkCycleCountKey);

	// Check if reached work cycle limit
	if (CurrentCycles >= WorkCyclesBeforeRest)
	{
		UE_LOG(LogTemp, Log, TEXT("Need rest: Work cycles reached (%d >= %d)"),
			CurrentCycles, WorkCyclesBeforeRest);
		return true;
	}

	// Random chance to need rest
	if (RandomRestChance > 0.0f && FMath::FRand() < RandomRestChance)
	{
		UE_LOG(LogTemp, Log, TEXT("Need rest: Random chance triggered"));
		return true;
	}

	return false;
}
