// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_ProcessResources.h"
#include "AIController.h"
#include "CraftsmanVillager.h"
#include "InventoryComponent.h"
#include "BaseBuilding.h"
#include "BuildingManager.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ProcessResources::UBTTask_ProcessResources()
{
	NodeName = "Process Resources";
	bUseAssignedWorkshop = true;
	MaxSearchDistance = 5000.0f;
	ProcessRadius = 200.0f;
	TargetBuildingKey = FName("TargetBuilding");

	// Default recipe: Wood -> Planks (Sawmill)
	Recipe.RequiredBuilding = EBuildingType::Sawmill;
	Recipe.CraftingTime = 3.0f;
	Recipe.InputResources.Add(FResourceStack(EResourceType::Wood, 2));
	Recipe.OutputResources.Add(FResourceStack(EResourceType::Planks, 1));

	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_ProcessResources::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ACraftsmanVillager* Craftsman = Cast<ACraftsmanVillager>(AIController->GetPawn());
	if (!Craftsman || !Craftsman->Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProcessResources: No craftsman or inventory"));
		return EBTNodeResult::Failed;
	}

	// Find target workshop
	ABaseBuilding* TargetWorkshop = nullptr;

	if (bUseAssignedWorkshop && Craftsman->IsAssigned())
	{
		TargetWorkshop = Craftsman->AssignedWorkshop;
	}
	else
	{
		// Find BuildingManager
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(Craftsman->GetWorld(), ABuildingManager::StaticClass(), FoundActors);

		if (FoundActors.Num() > 0)
		{
			ABuildingManager* BuildingManager = Cast<ABuildingManager>(FoundActors[0]);
			if (BuildingManager)
			{
				TargetWorkshop = BuildingManager->GetNearestBuilding(
					Craftsman->GetActorLocation(),
					Recipe.RequiredBuilding
				);
			}
		}
	}

	if (!TargetWorkshop)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No workshop found for processing"), *Craftsman->GetName());
		return EBTNodeResult::Failed;
	}

	// Check distance
	float Distance = FVector::Dist(Craftsman->GetActorLocation(), TargetWorkshop->GetBuildingLocation());
	if (Distance > MaxSearchDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Workshop too far (%f > %f)"),
			*Craftsman->GetName(), Distance, MaxSearchDistance);
		return EBTNodeResult::Failed;
	}

	// Check if we're close enough to process
	if (Distance > ProcessRadius)
	{
		// Store workshop in blackboard for movement task
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsObject(TargetBuildingKey, TargetWorkshop);
		}

		UE_LOG(LogTemp, Log, TEXT("%s: Not at workshop yet, need to move (%f > %f)"),
			*Craftsman->GetName(), Distance, ProcessRadius);
		return EBTNodeResult::Failed;
	}

	// Check if craftsman has required input resources
	for (const FResourceStack& Input : Recipe.InputResources)
	{
		if (!Craftsman->Inventory->HasResource(Input.ResourceType, Input.Quantity))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: Missing input resource %d x %d for crafting"),
				*Craftsman->GetName(), (int32)Input.ResourceType, Input.Quantity);
			return EBTNodeResult::Failed;
		}
	}

	// Consume input resources
	for (const FResourceStack& Input : Recipe.InputResources)
	{
		Craftsman->Inventory->RemoveResource(Input.ResourceType, Input.Quantity);
	}

	// Start processing
	float ActualCraftingTime = Recipe.CraftingTime;
	if (Craftsman->CraftingEfficiency > 0.0f)
	{
		ActualCraftingTime /= Craftsman->CraftingEfficiency;
	}

	float CurrentTime = Craftsman->GetWorld()->GetTimeSeconds();
	ProcessEndTimes.Add(&OwnerComp, CurrentTime + ActualCraftingTime);
	ProcessingWorkshops.Add(&OwnerComp, TargetWorkshop);

	// Update craftsman state
	Craftsman->CurrentState = EActorState::WORKING;

	UE_LOG(LogTemp, Log, TEXT("%s: Started processing at %s for %.2f seconds"),
		*Craftsman->GetName(), *TargetWorkshop->BuildingName, ActualCraftingTime);

	return EBTNodeResult::InProgress;
}

void UBTTask_ProcessResources::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	float* EndTime = ProcessEndTimes.Find(&OwnerComp);
	if (!EndTime)
	{
		ProcessingWorkshops.Remove(&OwnerComp);
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		ProcessEndTimes.Remove(&OwnerComp);
		ProcessingWorkshops.Remove(&OwnerComp);
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ACraftsmanVillager* Craftsman = Cast<ACraftsmanVillager>(AIController->GetPawn());
	if (!Craftsman)
	{
		ProcessEndTimes.Remove(&OwnerComp);
		ProcessingWorkshops.Remove(&OwnerComp);
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	float CurrentTime = Craftsman->GetWorld()->GetTimeSeconds();

	if (CurrentTime >= *EndTime)
	{
		// Processing complete - add output resources
		bool bSuccess = true;
		for (const FResourceStack& Output : Recipe.OutputResources)
		{
			int32 Added = Craftsman->Inventory->AddResource(Output.ResourceType, Output.Quantity);
			if (Added < Output.Quantity)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s: Inventory full, only added %d/%d of output"),
					*Craftsman->GetName(), Added, Output.Quantity);
				bSuccess = false;
			}
		}

		// Update state
		Craftsman->CurrentState = EActorState::IDLE;

		ABaseBuilding** Workshop = ProcessingWorkshops.Find(&OwnerComp);
		if (Workshop && *Workshop)
		{
			UE_LOG(LogTemp, Log, TEXT("%s: Finished processing at %s"),
				*Craftsman->GetName(), *(*Workshop)->BuildingName);
		}

		ProcessEndTimes.Remove(&OwnerComp);
		ProcessingWorkshops.Remove(&OwnerComp);
		FinishLatentTask(OwnerComp, bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
	}
}
