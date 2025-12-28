// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_FindWork.h"
#include "AIController.h"
#include "BaseVillager.h"
#include "InventoryComponent.h"
#include "ZoneManager.h"
#include "BuildingManager.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_FindWork::UBTTask_FindWork()
{
	NodeName = "Find Work";
	GatheringPriority = 1.0f;
	ProcessingPriority = 0.5f;
	WorkTypeKey = FName("WorkType");
	TargetZoneKey = FName("TargetZone");
	TargetBuildingKey = FName("TargetBuilding");
}

EBTNodeResult::Type UBTTask_FindWork::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ABaseVillager* Villager = Cast<ABaseVillager>(AIController->GetPawn());
	if (!Villager)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	// Simple work assignment logic
	// Priority 1: If inventory is full, deposit resources
	if (Villager->Inventory && Villager->Inventory->IsFull())
	{
		BlackboardComp->SetValueAsName(WorkTypeKey, FName("Deposit"));
		UE_LOG(LogTemp, Log, TEXT("%s: Work assigned - Deposit (inventory full)"), *Villager->GetName());
		return EBTNodeResult::Succeeded;
	}

	// Priority 2: If inventory is empty or low, gather resources
	if (!Villager->Inventory || Villager->Inventory->GetTotalItems() < 10)
	{
		BlackboardComp->SetValueAsName(WorkTypeKey, FName("Gather"));
		UE_LOG(LogTemp, Log, TEXT("%s: Work assigned - Gather (inventory empty/low)"), *Villager->GetName());
		return EBTNodeResult::Succeeded;
	}

	// Default: Gather more resources
	BlackboardComp->SetValueAsName(WorkTypeKey, FName("Gather"));
	UE_LOG(LogTemp, Log, TEXT("%s: Work assigned - Gather (default)"), *Villager->GetName());
	return EBTNodeResult::Succeeded;
}
