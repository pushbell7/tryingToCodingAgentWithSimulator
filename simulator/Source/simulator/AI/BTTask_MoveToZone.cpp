// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_MoveToZone.h"
#include "AIController.h"
#include "BaseVillager.h"
#include "ZoneManagerSubsystem.h"
#include "ZoneGrid.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_MoveToZone::UBTTask_MoveToZone()
{
	NodeName = "Move To Zone";
	TargetZoneType = ETerrainZone::Farmland;
	TargetLocationKey = FName("TargetLocation");
	AcceptanceRadius = 100.0f;
	MaxSearchDistance = 10000.0f;

	// This task uses the navigation system
	bNotifyTick = false;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_MoveToZone::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	// Get ZoneManagerSubsystem
	UZoneManagerSubsystem* ZoneManager = Villager->GetWorld()->GetSubsystem<UZoneManagerSubsystem>();
	if (!ZoneManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToZone: No ZoneManagerSubsystem found"));
		return EBTNodeResult::Failed;
	}

	// Get ZoneGrid
	AZoneGrid* ZoneGrid = ZoneManager->GetZoneGrid();
	if (!ZoneGrid)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToZone: No ZoneGrid found"));
		return EBTNodeResult::Failed;
	}

	// Check current location zone type
	FVector CurrentLocation = Villager->GetActorLocation();
	ETerrainZone CurrentZoneType = ZoneGrid->GetZoneTypeAtLocation(CurrentLocation);

	// Already in target zone type?
	if (CurrentZoneType == TargetZoneType)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Already in %s zone"),
			*Villager->GetName(), *UEnum::GetValueAsString(TargetZoneType));
		return EBTNodeResult::Succeeded;
	}

	// Find nearest cell of target zone type
	FVector TargetLocation = FVector::ZeroVector;
	float MinDistance = MaxSearchDistance;
	bool bFoundZone = false;

	// Simple search through all cells (could be optimized with spatial partitioning)
	for (const FZoneCellData& Cell : ZoneGrid->ZoneCells)
	{
		if (Cell.ZoneType == TargetZoneType)
		{
			float Distance = FVector::Dist(CurrentLocation, Cell.WorldPosition);
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				TargetLocation = Cell.WorldPosition;
				bFoundZone = true;
			}
		}
	}

	if (!bFoundZone)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No %s zone found within %.0f units"),
			*Villager->GetName(), *UEnum::GetValueAsString(TargetZoneType), MaxSearchDistance);
		return EBTNodeResult::Failed;
	}

	// Store target location in blackboard
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsVector(TargetLocationKey, TargetLocation);
	}

	// Move to target location
	EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(
		TargetLocation,
		AcceptanceRadius,
		true,  // bStopOnOverlap
		true,  // bUsePathfinding
		false, // bProjectDestinationToNavigation
		true,  // bCanStrafe
		nullptr // FilterClass
	);

	if (MoveResult == EPathFollowingRequestResult::RequestSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Moving to %s zone at %s (distance: %.0f)"),
			*Villager->GetName(), *UEnum::GetValueAsString(TargetZoneType),
			*TargetLocation.ToString(), MinDistance);
		return EBTNodeResult::InProgress;
	}
	else if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Already at %s zone"),
			*Villager->GetName(), *UEnum::GetValueAsString(TargetZoneType));
		return EBTNodeResult::Succeeded;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Failed to start moving to %s zone"),
			*Villager->GetName(), *UEnum::GetValueAsString(TargetZoneType));
		return EBTNodeResult::Failed;
	}
}
