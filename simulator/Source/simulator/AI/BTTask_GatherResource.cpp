// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_GatherResource.h"
#include "AIController.h"
#include "BaseVillager.h"
#include "BaseBuilding.h"
#include "InventoryComponent.h"
#include "ZoneManagerSubsystem.h"
#include "ZoneGrid.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_GatherResource::UBTTask_GatherResource()
{
	NodeName = "Gather Resource";
	TargetZoneType = ETerrainZone::Forest;
	GatherAmount = 10;
	bUseAssignedWorkplace = true;
}

EBTNodeResult::Type UBTTask_GatherResource::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ABaseVillager* Villager = Cast<ABaseVillager>(AIController->GetPawn());
	if (!Villager || !Villager->Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("GatherResource: No villager or inventory"));
		return EBTNodeResult::Failed;
	}

	// Check if inventory is full
	if (Villager->Inventory->IsFull())
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Inventory full, cannot gather"), *Villager->GetName());
		return EBTNodeResult::Failed;
	}

	// Get ZoneManagerSubsystem
	UZoneManagerSubsystem* ZoneManager = Villager->GetWorld()->GetSubsystem<UZoneManagerSubsystem>();
	if (!ZoneManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("GatherResource: No ZoneManagerSubsystem found"));
		return EBTNodeResult::Failed;
	}

	// Get ZoneGrid
	AZoneGrid* ZoneGrid = ZoneManager->GetZoneGrid();
	if (!ZoneGrid)
	{
		UE_LOG(LogTemp, Warning, TEXT("GatherResource: No ZoneGrid found"));
		return EBTNodeResult::Failed;
	}

	// Determine work location (from workplace or current location)
	FVector WorkLocation = Villager->GetActorLocation();

	if (bUseAssignedWorkplace && Villager->AssignedWorkplace)
	{
		WorkLocation = Villager->AssignedWorkplace->GetBuildingLocation();
		UE_LOG(LogTemp, Log, TEXT("%s: Using workplace '%s' location"),
			*Villager->GetName(), *Villager->AssignedWorkplace->BuildingName);
	}

	// Get zone type at work location
	ETerrainZone WorkZoneType = ZoneGrid->GetZoneTypeAtLocation(WorkLocation);

	// Verify zone type matches (if we care about specific type)
	if (WorkZoneType != TargetZoneType && TargetZoneType != ETerrainZone::Farmland)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Work location has wrong zone type (%s, expected %s)"),
			*Villager->GetName(),
			*UEnum::GetValueAsString(WorkZoneType),
			*UEnum::GetValueAsString(TargetZoneType));
		return EBTNodeResult::Failed;
	}

	// Determine resource type from zone type
	EResourceType ResourceType = EResourceType::Food;

	switch (WorkZoneType)
	{
	case ETerrainZone::Forest:
		ResourceType = EResourceType::Wood;
		break;
	case ETerrainZone::Mountain:
		ResourceType = EResourceType::Stone;
		break;
	case ETerrainZone::Farmland:
	case ETerrainZone::Pasture:
		ResourceType = EResourceType::Food;
		break;
	case ETerrainZone::Water:
		// Water doesn't produce gatherable resources directly
		UE_LOG(LogTemp, Warning, TEXT("%s: Cannot gather from Water zone"), *Villager->GetName());
		return EBTNodeResult::Failed;
	case ETerrainZone::Settlement:
		// Settlements don't produce raw resources
		UE_LOG(LogTemp, Warning, TEXT("%s: Cannot gather from Settlement zone"), *Villager->GetName());
		return EBTNodeResult::Failed;
	default:
		ResourceType = EResourceType::Food;
		break;
	}

	// Gather resources (simplified - just add to inventory)
	// TODO: Add resource depletion/regeneration system to ZoneGrid cells
	int32 AmountAdded = Villager->Inventory->AddResource(ResourceType, GatherAmount);

	if (AmountAdded > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Gathered %d x %s from %s zone"),
			*Villager->GetName(),
			AmountAdded,
			*UEnum::GetValueAsString(ResourceType),
			*UEnum::GetValueAsString(WorkZoneType));
		return EBTNodeResult::Succeeded;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Inventory full, couldn't add resources"), *Villager->GetName());
		return EBTNodeResult::Failed;
	}
}
