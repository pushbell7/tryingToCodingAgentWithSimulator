// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_MoveToBuilding.h"
#include "AIController.h"
#include "BaseBuilding.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_MoveToBuilding::UBTTask_MoveToBuilding()
{
	NodeName = "Move To Building";
	TargetBuildingKey = FName("TargetBuilding");
	AcceptanceRadius = 200.0f;

	// This task uses the navigation system
	bNotifyTick = false;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_MoveToBuilding::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	// Get target building from blackboard
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	UObject* TargetObject = BlackboardComp->GetValueAsObject(TargetBuildingKey);
	ABaseBuilding* TargetBuilding = Cast<ABaseBuilding>(TargetObject);

	if (!TargetBuilding)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No target building in blackboard"), *ControlledPawn->GetName());
		return EBTNodeResult::Failed;
	}

	// Get target location
	FVector TargetLocation = TargetBuilding->GetBuildingLocation();

	// Check if already close enough
	float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), TargetLocation);
	if (Distance <= AcceptanceRadius)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Already at building %s"),
			*ControlledPawn->GetName(), *TargetBuilding->BuildingName);
		return EBTNodeResult::Succeeded;
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
		UE_LOG(LogTemp, Log, TEXT("%s: Moving to building %s at %s"),
			*ControlledPawn->GetName(), *TargetBuilding->BuildingName, *TargetLocation.ToString());
		return EBTNodeResult::InProgress;
	}
	else if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Already at building %s"),
			*ControlledPawn->GetName(), *TargetBuilding->BuildingName);
		return EBTNodeResult::Succeeded;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Failed to start moving to building %s"),
			*ControlledPawn->GetName(), *TargetBuilding->BuildingName);
		return EBTNodeResult::Failed;
	}
}
