// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTService_UpdateWorkState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "BaseVillager.h"

UBTService_UpdateWorkState::UBTService_UpdateWorkState()
{
	NodeName = "Update Work State";
	WorkCycleCountKey = FName("WorkCycleCount");
	LastWorkResultKey = FName("LastWorkResult");
	bIncrementOnSuccess = true;

	// Update every 0.5 seconds
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UBTService_UpdateWorkState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return;
	}

	ABaseVillager* Villager = Cast<ABaseVillager>(AIController->GetPawn());
	if (!Villager)
	{
		return;
	}

	// Update villager state in blackboard
	BlackboardComp->SetValueAsEnum(FName("CurrentState"), (uint8)Villager->CurrentState);

	// Check if villager just completed work (state changed from WORKING to IDLE)
	static TMap<UBehaviorTreeComponent*, EActorState> PreviousStates;

	EActorState* PrevState = PreviousStates.Find(&OwnerComp);
	if (PrevState && *PrevState == EActorState::WORKING && Villager->CurrentState == EActorState::IDLE)
	{
		// Work completed, increment cycle counter
		if (bIncrementOnSuccess)
		{
			int32 CurrentCycles = BlackboardComp->GetValueAsInt(WorkCycleCountKey);
			BlackboardComp->SetValueAsInt(WorkCycleCountKey, CurrentCycles + 1);

			UE_LOG(LogTemp, Log, TEXT("%s: Work cycle completed (%d total)"),
				*Villager->GetName(), CurrentCycles + 1);
		}
	}

	// Store current state for next tick
	PreviousStates.Add(&OwnerComp, Villager->CurrentState);
}
