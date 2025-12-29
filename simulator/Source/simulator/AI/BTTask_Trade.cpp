// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_Trade.h"
#include "AIController.h"
#include "BaseVillager.h"
#include "MerchantVillager.h"
#include "InventoryComponent.h"
#include "BaseBuilding.h"
#include "BuildingManagerSubsystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EngineUtils.h"

UBTTask_Trade::UBTTask_Trade()
{
	NodeName = "Trade at Market";
	ResourceType = EResourceType::Food;
	Quantity = 10;
	bBuying = false;
	MaxSearchDistance = 5000.0f;
	TradeRadius = 300.0f;
	TargetBuildingKey = FName("TargetBuilding");
}

EBTNodeResult::Type UBTTask_Trade::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ABaseVillager* Villager = Cast<ABaseVillager>(AIController->GetPawn());
	if (!Villager || !Villager->Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trade: No villager or inventory"));
		return EBTNodeResult::Failed;
	}

	// Get BuildingManagerSubsystem
	UBuildingManagerSubsystem* BuildingManager = Villager->GetWorld()->GetSubsystem<UBuildingManagerSubsystem>();
	if (!BuildingManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trade: No BuildingManagerSubsystem found"));
		return EBTNodeResult::Failed;
	}

	// Find nearest market
	ABaseBuilding* Market = BuildingManager->GetNearestBuilding(Villager->GetActorLocation(), EBuildingType::Market);

	if (!Market)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No market found"), *Villager->GetName());
		return EBTNodeResult::Failed;
	}

	// Check distance
	float Distance = FVector::Dist(Villager->GetActorLocation(), Market->GetBuildingLocation());
	if (Distance > MaxSearchDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Market too far (%f > %f)"),
			*Villager->GetName(), Distance, MaxSearchDistance);
		return EBTNodeResult::Failed;
	}

	// Check if we're close enough to trade
	if (Distance > TradeRadius)
	{
		// Store market in blackboard for movement task
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsObject(TargetBuildingKey, Market);
		}

		UE_LOG(LogTemp, Log, TEXT("%s: Not at market yet, need to move (%f > %f)"),
			*Villager->GetName(), Distance, TradeRadius);
		return EBTNodeResult::Failed;
	}

	// Find a merchant at the market
	AMerchantVillager* Merchant = nullptr;
	for (TActorIterator<AMerchantVillager> It(Villager->GetWorld()); It; ++It)
	{
		AMerchantVillager* PotentialMerchant = *It;
		if (PotentialMerchant && PotentialMerchant->AssignedMarket == Market)
		{
			Merchant = PotentialMerchant;
			break;
		}
	}

	if (!Merchant)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No merchant found at market"), *Villager->GetName());
		return EBTNodeResult::Failed;
	}

	// Execute trade
	if (bBuying)
	{
		// Villager buying from merchant
		int32 Price = Merchant->GetBuyPrice(ResourceType);
		if (Price == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: Merchant not selling %d"),
				*Villager->GetName(), (int32)ResourceType);
			return EBTNodeResult::Failed;
		}

		// For now, assume villagers have gold in inventory as Gold resource
		int32 TotalCost = Price * Quantity;
		if (!Villager->Inventory->HasResource(EResourceType::Gold, TotalCost))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: Not enough gold (%d needed)"),
				*Villager->GetName(), TotalCost);
			return EBTNodeResult::Failed;
		}

		if (Merchant->ExecuteTrade(ResourceType, Quantity, true))
		{
			// Remove gold, add resource
			Villager->Inventory->RemoveResource(EResourceType::Gold, TotalCost);
			Villager->Inventory->AddResource(ResourceType, Quantity);

			UE_LOG(LogTemp, Log, TEXT("%s: Bought %d x %d for %d gold from %s"),
				*Villager->GetName(), (int32)ResourceType, Quantity, TotalCost, *Merchant->VillagerName);
			return EBTNodeResult::Succeeded;
		}
	}
	else
	{
		// Villager selling to merchant
		if (!Villager->Inventory->HasResource(ResourceType, Quantity))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: Not enough %d to sell (has %d, needs %d)"),
				*Villager->GetName(), (int32)ResourceType,
				Villager->Inventory->GetResourceQuantity(ResourceType), Quantity);
			return EBTNodeResult::Failed;
		}

		int32 Price = Merchant->GetSellPrice(ResourceType);
		if (Price == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: Merchant not buying %d"),
				*Villager->GetName(), (int32)ResourceType);
			return EBTNodeResult::Failed;
		}

		if (Merchant->ExecuteTrade(ResourceType, Quantity, false))
		{
			// Remove resource, add gold
			int32 TotalEarned = Price * Quantity;
			Villager->Inventory->RemoveResource(ResourceType, Quantity);
			Villager->Inventory->AddResource(EResourceType::Gold, TotalEarned);

			UE_LOG(LogTemp, Log, TEXT("%s: Sold %d x %d for %d gold to %s"),
				*Villager->GetName(), (int32)ResourceType, Quantity, TotalEarned, *Merchant->VillagerName);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}
