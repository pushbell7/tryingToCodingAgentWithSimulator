// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_Rest.h"
#include "AIController.h"
#include "BaseVillager.h"
#include "BaseBuilding.h"
#include "House.h"
#include "BuildingManagerSubsystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Rest::UBTTask_Rest()
{
	NodeName = "Rest at Home";
	RestDuration = 5.0f;
	RandomDeviation = 2.0f;
	MaxSearchDistance = 5000.0f;
	RestRadius = 300.0f;
	TargetBuildingKey = FName("TargetBuilding");

	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_Rest::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	// Get BuildingManagerSubsystem
	UBuildingManagerSubsystem* BuildingManager = Villager->GetWorld()->GetSubsystem<UBuildingManagerSubsystem>();
	if (!BuildingManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Rest: No BuildingManagerSubsystem found"));
		return EBTNodeResult::Failed;
	}

	// Use assigned home if available, otherwise find nearest house
	ABaseBuilding* TargetHouse = Cast<ABaseBuilding>(Villager->AssignedHome);

	if (!TargetHouse)
	{
		// Fallback to nearest house if no home assigned
		TargetHouse = BuildingManager->GetNearestBuilding(Villager->GetActorLocation(), EBuildingType::House);

		if (!TargetHouse)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: No house found for resting"), *Villager->GetName());
			return EBTNodeResult::Failed;
		}

		UE_LOG(LogTemp, Log, TEXT("%s: No assigned home, using nearest house '%s'"),
			*Villager->GetName(), *TargetHouse->BuildingName);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Using assigned home '%s'"),
			*Villager->GetName(), *TargetHouse->BuildingName);
	}

	// Check distance
	float Distance = FVector::Dist(Villager->GetActorLocation(), TargetHouse->GetBuildingLocation());
	if (Distance > MaxSearchDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: House too far (%f > %f)"), *Villager->GetName(), Distance, MaxSearchDistance);
		return EBTNodeResult::Failed;
	}

	// Check if we're close enough to rest
	if (Distance > RestRadius)
	{
		// Store house in blackboard for movement task
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsObject(TargetBuildingKey, TargetHouse);
		}

		UE_LOG(LogTemp, Log, TEXT("%s: Not at house yet, need to move (%f > %f)"),
			*Villager->GetName(), Distance, RestRadius);
		return EBTNodeResult::Failed; // Need to move to house first
	}

	// Start resting - calculate actual rest time with random deviation
	float ActualRestTime = RestDuration;
	if (RandomDeviation > 0.0f)
	{
		ActualRestTime += FMath::RandRange(-RandomDeviation, RandomDeviation);
		ActualRestTime = FMath::Max(1.0f, ActualRestTime); // Minimum 1 second
	}

	// Store end time
	float CurrentTime = Villager->GetWorld()->GetTimeSeconds();
	RestEndTimes.Add(&OwnerComp, CurrentTime + ActualRestTime);

	// Update villager state to resting
	Villager->CurrentState = EActorState::RESTING;

	UE_LOG(LogTemp, Log, TEXT("%s: Started resting at %s for %.2f seconds"),
		*Villager->GetName(), *TargetHouse->BuildingName, ActualRestTime);

	return EBTNodeResult::InProgress;
}

void UBTTask_Rest::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	float* EndTime = RestEndTimes.Find(&OwnerComp);
	if (!EndTime)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		RestEndTimes.Remove(&OwnerComp);
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ABaseVillager* Villager = Cast<ABaseVillager>(AIController->GetPawn());
	if (!Villager)
	{
		RestEndTimes.Remove(&OwnerComp);
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	float CurrentTime = Villager->GetWorld()->GetTimeSeconds();

	if (CurrentTime >= *EndTime)
	{
		// Rest is complete
		Villager->CurrentState = EActorState::IDLE;

		UE_LOG(LogTemp, Log, TEXT("%s: Finished resting, feeling refreshed!"), *Villager->GetName());

		RestEndTimes.Remove(&OwnerComp);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
