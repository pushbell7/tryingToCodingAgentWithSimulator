// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_GatherResource.h"
#include "AIController.h"
#include "BaseVillager.h"
#include "InventoryComponent.h"
#include "TerrainZone.h"
#include "ZoneManager.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_GatherResource::UBTTask_GatherResource()
{
	NodeName = "Gather Resource";
	TargetZoneType = ETerrainZone::Forest;
	GatherAmount = 10;
	MaxSearchDistance = 5000.0f;
	TargetZoneKey = FName("TargetZone");
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

	// Find ZoneManager
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(Villager->GetWorld(), AZoneManager::StaticClass(), FoundActors);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("GatherResource: No ZoneManager found"));
		return EBTNodeResult::Failed;
	}

	AZoneManager* ZoneManager = Cast<AZoneManager>(FoundActors[0]);
	if (!ZoneManager)
	{
		return EBTNodeResult::Failed;
	}

	// Use assigned work zone if available and matches target type, otherwise find nearest zone
	ATerrainZone* TargetZone = nullptr;

	if (Villager->AssignedWorkZone && Villager->AssignedWorkZone->ZoneType == TargetZoneType)
	{
		TargetZone = Villager->AssignedWorkZone;
		UE_LOG(LogTemp, Log, TEXT("%s: Using assigned work zone '%s'"),
			*Villager->GetName(), *TargetZone->ZoneName);
	}
	else
	{
		// Fallback to nearest zone if no work zone assigned or type mismatch
		TargetZone = ZoneManager->GetNearestZone(Villager->GetActorLocation(), TargetZoneType);

		if (!TargetZone)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: No zone of type %d found"), *Villager->GetName(), (int32)TargetZoneType);
			return EBTNodeResult::Failed;
		}

		UE_LOG(LogTemp, Log, TEXT("%s: No assigned work zone or type mismatch, using nearest zone '%s'"),
			*Villager->GetName(), *TargetZone->ZoneName);
	}

	// Check distance
	float Distance = FVector::Dist(Villager->GetActorLocation(), TargetZone->GetZoneCenter());
	if (Distance > MaxSearchDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Zone too far (%f > %f)"), *Villager->GetName(), Distance, MaxSearchDistance);
		return EBTNodeResult::Failed;
	}

	// Check if villager is in the zone
	if (!TargetZone->IsActorInZone(Villager))
	{
		// Store zone in blackboard for movement task
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsObject(TargetZoneKey, TargetZone);
		}

		UE_LOG(LogTemp, Log, TEXT("%s: Not in zone yet, need to move to %s"),
			*Villager->GetName(), *TargetZone->GetName());
		return EBTNodeResult::Failed; // Need to move to zone first
	}

	// Gather resources from zone
	int32 AmountGathered = TargetZone->GatherResources(GatherAmount);

	if (AmountGathered <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Zone depleted or cannot produce"), *Villager->GetName());
		return EBTNodeResult::Failed;
	}

	// Add to inventory
	EResourceType ResourceType = TargetZone->GetProducedResourceType();
	int32 AmountAdded = Villager->Inventory->AddResource(ResourceType, AmountGathered);

	if (AmountAdded > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Gathered %d x %d from %s"),
			*Villager->GetName(), (int32)ResourceType, AmountAdded, *TargetZone->GetName());
		return EBTNodeResult::Succeeded;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Inventory full, couldn't add resources"), *Villager->GetName());
		return EBTNodeResult::Failed;
	}
}
