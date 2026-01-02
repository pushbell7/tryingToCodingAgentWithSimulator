// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_MoveToTarget.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BaseVillager.h"
#include "BaseBuilding.h"

UBTTask_MoveToTarget::UBTTask_MoveToTarget()
{
	NodeName = "Move To Target";
	bNotifyTick = true;
	bNotifyTaskFinished = true;

	TargetKey = FName("TargetLocation");
	AcceptanceRadius = 150.0f;
	bShouldRun = false;
}

EBTNodeResult::Type UBTTask_MoveToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToTarget: No AI Controller"));
		return EBTNodeResult::Failed;
	}

	ABaseVillager* Villager = Cast<ABaseVillager>(AIController->GetPawn());
	if (!Villager)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToTarget: No villager pawn"));
		return EBTNodeResult::Failed;
	}

	// Get target location
	FVector TargetLoc;
	if (!GetTargetLocation(OwnerComp, TargetLoc))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: MoveToTarget - No valid target location"), *Villager->GetName());
		return EBTNodeResult::Failed;
	}

	// Check if already at target
	float Distance = FVector::Dist(Villager->GetActorLocation(), TargetLoc);
	if (Distance <= AcceptanceRadius)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Already at target (%.0f <= %.0f)"),
			*Villager->GetName(), Distance, AcceptanceRadius);
		return EBTNodeResult::Succeeded;
	}

	// Set movement speed
	UCharacterMovementComponent* Movement = Villager->GetCharacterMovement();
	if (Movement)
	{
		Movement->MaxWalkSpeed = bShouldRun ? Villager->RunSpeed : Villager->WalkSpeed;
	}

	// Start moving using AI navigation
	EPathFollowingRequestResult::Type Result = AIController->MoveToLocation(
		TargetLoc,
		AcceptanceRadius,
		true,  // bStopOnOverlap
		true,  // bUsePathfinding
		false, // bProjectDestinationToNavigation
		true   // bCanStrafe
	);

	if (Result == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: MoveToTarget pathfinding failed to %s"),
			*Villager->GetName(), *TargetLoc.ToString());
		return EBTNodeResult::Failed;
	}

	if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Already at goal"), *Villager->GetName());
		return EBTNodeResult::Succeeded;
	}

	// Movement started successfully
	UE_LOG(LogTemp, Log, TEXT("%s: Moving to %s (distance: %.0f)"),
		*Villager->GetName(), *TargetLoc.ToString(), Distance);

	Villager->CurrentState = EActorState::MOVING;
	return EBTNodeResult::InProgress;
}

void UBTTask_MoveToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Check movement status
	EPathFollowingStatus::Type Status = AIController->GetMoveStatus();

	switch (Status)
	{
	case EPathFollowingStatus::Moving:
		// Still moving, continue
		break;

	case EPathFollowingStatus::Idle:
		// Movement stopped but not reached goal - probably failed
		UE_LOG(LogTemp, Warning, TEXT("MoveToTarget: Movement became idle"));
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		break;

	case EPathFollowingStatus::Waiting:
	case EPathFollowingStatus::Paused:
		// Paused, keep waiting
		break;
	}
}

void UBTTask_MoveToTarget::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		ABaseVillager* Villager = Cast<ABaseVillager>(AIController->GetPawn());
		if (Villager)
		{
			if (TaskResult == EBTNodeResult::Succeeded)
			{
				UE_LOG(LogTemp, Log, TEXT("%s: Reached destination"), *Villager->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s: Movement failed"), *Villager->GetName());
			}

			Villager->CurrentState = EActorState::IDLE;
		}
	}

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

bool UBTTask_MoveToTarget::GetTargetLocation(UBehaviorTreeComponent& OwnerComp, FVector& OutLocation)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return false;
	}

	// Try to get as object first (Building or Actor)
	UObject* TargetObject = BlackboardComp->GetValueAsObject(TargetKey);
	if (TargetObject)
	{
		// Check if it's a Building
		ABaseBuilding* Building = Cast<ABaseBuilding>(TargetObject);
		if (Building)
		{
			OutLocation = Building->GetBuildingLocation();
			return true;
		}

		// Check if it's an Actor
		AActor* Actor = Cast<AActor>(TargetObject);
		if (Actor)
		{
			OutLocation = Actor->GetActorLocation();
			return true;
		}
	}

	// Try to get as vector
	OutLocation = BlackboardComp->GetValueAsVector(TargetKey);
	return !OutLocation.IsZero();
}
