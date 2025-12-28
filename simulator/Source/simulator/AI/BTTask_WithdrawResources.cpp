// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_WithdrawResources.h"
#include "AIController.h"
#include "BaseVillager.h"
#include "InventoryComponent.h"
#include "BaseBuilding.h"
#include "BuildingManager.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_WithdrawResources::UBTTask_WithdrawResources()
{
	NodeName = "Withdraw Resources";
	ResourceType = EResourceType::Food;
	WithdrawAmount = 10;
	MaxSearchDistance = 10000.0f;
	TargetBuildingKey = FName("TargetBuilding");
	WithdrawRadius = 200.0f;
}

EBTNodeResult::Type UBTTask_WithdrawResources::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ABaseVillager* Villager = Cast<ABaseVillager>(AIController->GetPawn());
	if (!Villager || !Villager->Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("WithdrawResources: No villager or inventory"));
		return EBTNodeResult::Failed;
	}

	// Check if villager has room in inventory
	if (Villager->Inventory->IsFull())
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Inventory full, cannot withdraw"), *Villager->GetName());
		return EBTNodeResult::Failed;
	}

	// Find BuildingManager
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(Villager->GetWorld(), ABuildingManager::StaticClass(), FoundActors);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("WithdrawResources: No BuildingManager found"));
		return EBTNodeResult::Failed;
	}

	ABuildingManager* BuildingManager = Cast<ABuildingManager>(FoundActors[0]);
	if (!BuildingManager)
	{
		return EBTNodeResult::Failed;
	}

	// Find storage buildings with the desired resource
	TArray<ABaseBuilding*> StorageBuildings = BuildingManager->GetAllStorageBuildings();
	ABaseBuilding* TargetStorage = nullptr;
	float NearestDistance = FLT_MAX;

	for (ABaseBuilding* Storage : StorageBuildings)
	{
		if (Storage && Storage->Inventory && Storage->Inventory->HasResource(ResourceType, WithdrawAmount))
		{
			float Distance = FVector::Dist(Villager->GetActorLocation(), Storage->GetBuildingLocation());
			if (Distance < NearestDistance && Distance <= MaxSearchDistance)
			{
				NearestDistance = Distance;
				TargetStorage = Storage;
			}
		}
	}

	if (!TargetStorage)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No storage with %d x %d found"),
			*Villager->GetName(), (int32)ResourceType, WithdrawAmount);
		return EBTNodeResult::Failed;
	}

	// Check if we're close enough to withdraw
	if (NearestDistance > WithdrawRadius)
	{
		// Store building in blackboard for movement task
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsObject(TargetBuildingKey, TargetStorage);
		}

		UE_LOG(LogTemp, Log, TEXT("%s: Not close enough to storage, need to move (%f > %f)"),
			*Villager->GetName(), NearestDistance, WithdrawRadius);
		return EBTNodeResult::Failed; // Need to move closer first
	}

	// Withdraw resources from storage to villager
	int32 RemovedAmount = TargetStorage->Inventory->RemoveResource(ResourceType, WithdrawAmount);

	if (RemovedAmount > 0)
	{
		int32 AddedAmount = Villager->Inventory->AddResource(ResourceType, RemovedAmount);

		// If villager couldn't accept all, put remainder back in storage
		if (AddedAmount < RemovedAmount)
		{
			int32 Remainder = RemovedAmount - AddedAmount;
			TargetStorage->Inventory->AddResource(ResourceType, Remainder);

			UE_LOG(LogTemp, Warning, TEXT("%s: Inventory full, %d x %d returned to storage"),
				*Villager->GetName(), (int32)ResourceType, Remainder);
		}

		UE_LOG(LogTemp, Log, TEXT("%s: Withdrew %d x %d from %s"),
			*Villager->GetName(), (int32)ResourceType, AddedAmount, *TargetStorage->BuildingName);
		return EBTNodeResult::Succeeded;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Failed to withdraw resources from %s"),
			*Villager->GetName(), *TargetStorage->BuildingName);
		return EBTNodeResult::Failed;
	}
}
