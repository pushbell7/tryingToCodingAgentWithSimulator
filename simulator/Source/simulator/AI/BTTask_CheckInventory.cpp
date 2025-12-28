// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_CheckInventory.h"
#include "AIController.h"
#include "BaseVillager.h"
#include "InventoryComponent.h"

UBTTask_CheckInventory::UBTTask_CheckInventory()
{
	NodeName = "Check Inventory";
	CheckType = EInventoryCheckType::IsFull;
	MinimumItems = 10;
	ResourceToCheck = EResourceType::Food;
	MinimumQuantity = 1;
}

EBTNodeResult::Type UBTTask_CheckInventory::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ABaseVillager* Villager = Cast<ABaseVillager>(AIController->GetPawn());
	if (!Villager || !Villager->Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("CheckInventory: No villager or inventory"));
		return EBTNodeResult::Failed;
	}

	bool CheckPassed = false;

	switch (CheckType)
	{
	case EInventoryCheckType::IsFull:
		CheckPassed = Villager->Inventory->IsFull();
		UE_LOG(LogTemp, Log, TEXT("%s: Inventory full check = %s"),
			*Villager->GetName(), CheckPassed ? TEXT("true") : TEXT("false"));
		break;

	case EInventoryCheckType::IsEmpty:
		CheckPassed = (Villager->Inventory->GetTotalItems() == 0);
		UE_LOG(LogTemp, Log, TEXT("%s: Inventory empty check = %s"),
			*Villager->GetName(), CheckPassed ? TEXT("true") : TEXT("false"));
		break;

	case EInventoryCheckType::HasMinimum:
		CheckPassed = (Villager->Inventory->GetTotalItems() >= MinimumItems);
		UE_LOG(LogTemp, Log, TEXT("%s: Inventory has minimum %d items = %s (current: %d)"),
			*Villager->GetName(), MinimumItems, CheckPassed ? TEXT("true") : TEXT("false"),
			Villager->Inventory->GetTotalItems());
		break;

	case EInventoryCheckType::HasResource:
		CheckPassed = Villager->Inventory->HasResource(ResourceToCheck, MinimumQuantity);
		UE_LOG(LogTemp, Log, TEXT("%s: Has %d x %d = %s (current: %d)"),
			*Villager->GetName(), (int32)ResourceToCheck, MinimumQuantity,
			CheckPassed ? TEXT("true") : TEXT("false"),
			Villager->Inventory->GetResourceQuantity(ResourceToCheck));
		break;

	default:
		UE_LOG(LogTemp, Warning, TEXT("%s: Unknown inventory check type"), *Villager->GetName());
		return EBTNodeResult::Failed;
	}

	return CheckPassed ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
