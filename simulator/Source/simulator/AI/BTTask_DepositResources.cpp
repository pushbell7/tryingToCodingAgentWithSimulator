// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_DepositResources.h"
#include "AIController.h"
#include "BaseVillager.h"
#include "InventoryComponent.h"
#include "BaseBuilding.h"
#include "BuildingManagerSubsystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_DepositResources::UBTTask_DepositResources()
{
	NodeName = "Deposit Resources";
	MaxSearchDistance = 10000.0f;
	TargetBuildingKey = FName("TargetBuilding");
	DepositRadius = 200.0f;
}

EBTNodeResult::Type UBTTask_DepositResources::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ABaseVillager* Villager = Cast<ABaseVillager>(AIController->GetPawn());
	if (!Villager || !Villager->Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("DepositResources: No villager or inventory"));
		return EBTNodeResult::Failed;
	}

	// Check if villager has any resources to deposit
	if (Villager->Inventory->GetTotalItems() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: No resources to deposit"), *Villager->GetName());
		return EBTNodeResult::Failed;
	}

	// Get BuildingManagerSubsystem
	UBuildingManagerSubsystem* BuildingManager = Villager->GetWorld()->GetSubsystem<UBuildingManagerSubsystem>();
	if (!BuildingManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("DepositResources: No BuildingManagerSubsystem found"));
		return EBTNodeResult::Failed;
	}

	// Find nearest available storage building
	ABaseBuilding* TargetStorage = BuildingManager->GetNearestAvailableStorage(Villager->GetActorLocation());

	if (!TargetStorage)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No storage building available"), *Villager->GetName());
		return EBTNodeResult::Failed;
	}

	// Check distance
	float Distance = FVector::Dist(Villager->GetActorLocation(), TargetStorage->GetBuildingLocation());
	if (Distance > MaxSearchDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Storage too far (%f > %f)"), *Villager->GetName(), Distance, MaxSearchDistance);
		return EBTNodeResult::Failed;
	}

	// Check if we're close enough to deposit
	if (Distance > DepositRadius)
	{
		// Store building in blackboard for movement task
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsObject(TargetBuildingKey, TargetStorage);
		}

		UE_LOG(LogTemp, Log, TEXT("%s: Not close enough to storage, need to move (%f > %f)"),
			*Villager->GetName(), Distance, DepositRadius);
		return EBTNodeResult::Failed; // Need to move closer first
	}

	// Transfer all resources from villager to storage
	TArray<FResourceStack> VillagerResources = Villager->Inventory->GetAllResources();
	int32 TotalDeposited = 0;

	for (const FResourceStack& Stack : VillagerResources)
	{
		if (Stack.Quantity > 0)
		{
			// Remove from villager
			int32 RemovedAmount = Villager->Inventory->RemoveResource(Stack.ResourceType, Stack.Quantity);

			// Add to storage
			int32 AddedAmount = TargetStorage->Inventory->AddResource(Stack.ResourceType, RemovedAmount);

			TotalDeposited += AddedAmount;

			// If storage couldn't accept all, put remainder back in villager
			if (AddedAmount < RemovedAmount)
			{
				int32 Remainder = RemovedAmount - AddedAmount;
				Villager->Inventory->AddResource(Stack.ResourceType, Remainder);

				UE_LOG(LogTemp, Warning, TEXT("%s: Storage full, %d x %d returned to villager"),
					*Villager->GetName(), (int32)Stack.ResourceType, Remainder);
			}
		}
	}

	if (TotalDeposited > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Deposited %d items to %s"),
			*Villager->GetName(), TotalDeposited, *TargetStorage->BuildingName);
		return EBTNodeResult::Succeeded;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Failed to deposit any resources"), *Villager->GetName());
		return EBTNodeResult::Failed;
	}
}
