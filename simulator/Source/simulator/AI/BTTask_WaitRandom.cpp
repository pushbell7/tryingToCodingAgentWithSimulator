// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_WaitRandom.h"

UBTTask_WaitRandom::UBTTask_WaitRandom()
{
	NodeName = "Wait Random";
	MinWaitTime = 2.0f;
	MaxWaitTime = 5.0f;
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_WaitRandom::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FWaitTaskMemory* Memory = CastInstanceNodeMemory<FWaitTaskMemory>(NodeMemory);
	Memory->RemainingTime = FMath::RandRange(MinWaitTime, MaxWaitTime);

	return EBTNodeResult::InProgress;
}

void UBTTask_WaitRandom::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FWaitTaskMemory* Memory = CastInstanceNodeMemory<FWaitTaskMemory>(NodeMemory);
	Memory->RemainingTime -= DeltaSeconds;

	if (Memory->RemainingTime <= 0.0f)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

uint16 UBTTask_WaitRandom::GetInstanceMemorySize() const
{
	return sizeof(FWaitTaskMemory);
}
