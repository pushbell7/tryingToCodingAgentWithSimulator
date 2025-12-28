// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_MoveToZone.h"
#include "AIController.h"
#include "TerrainZone.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_MoveToZone::UBTTask_MoveToZone()
{
	NodeName = "Move To Zone";
	TargetZoneKey = FName("TargetZone");
	AcceptanceRadius = 100.0f;
	bMoveToCenter = false; // Just enter the zone by default

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

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	// Get target zone from blackboard
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	UObject* TargetObject = BlackboardComp->GetValueAsObject(TargetZoneKey);
	ATerrainZone* TargetZone = Cast<ATerrainZone>(TargetObject);

	if (!TargetZone)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No target zone in blackboard"), *ControlledPawn->GetName());
		return EBTNodeResult::Failed;
	}

	// Check if already in zone
	if (TargetZone->IsActorInZone(ControlledPawn))
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Already in target zone %s"),
			*ControlledPawn->GetName(), *TargetZone->GetName());
		return EBTNodeResult::Succeeded;
	}

	// Determine target location
	FVector TargetLocation;
	if (bMoveToCenter)
	{
		TargetLocation = TargetZone->GetZoneCenter();
	}
	else
	{
		// Move towards center but don't require reaching it exactly
		TargetLocation = TargetZone->GetZoneCenter();
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
		UE_LOG(LogTemp, Log, TEXT("%s: Moving to zone %s at %s"),
			*ControlledPawn->GetName(), *TargetZone->GetName(), *TargetLocation.ToString());
		return EBTNodeResult::InProgress;
	}
	else if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Already at zone %s"),
			*ControlledPawn->GetName(), *TargetZone->GetName());
		return EBTNodeResult::Succeeded;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Failed to start moving to zone %s"),
			*ControlledPawn->GetName(), *TargetZone->GetName());
		return EBTNodeResult::Failed;
	}
}
