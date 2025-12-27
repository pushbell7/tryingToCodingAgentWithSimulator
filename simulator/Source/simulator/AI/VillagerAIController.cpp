// Copyright Epic Games, Inc. All Rights Reserved.

#include "VillagerAIController.h"
#include "BaseVillager.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "VillagerBlackboardKeys.h"

AVillagerAIController::AVillagerAIController()
{
	// Initialize Blackboard component
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

void AVillagerAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AVillagerAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledVillager = Cast<ABaseVillager>(InPawn);

	if (ControlledVillager)
	{
		// Initialize Blackboard if asset is set
		if (BlackboardAsset)
		{
			Blackboard->InitializeBlackboard(*BlackboardAsset);

			// Set self reference
			Blackboard->SetValueAsObject(VillagerBlackboardKeys::SelfActor, ControlledVillager);
		}

		// Run Behavior Tree if asset is set
		if (BehaviorTreeAsset)
		{
			RunBehaviorTree(BehaviorTreeAsset);
			UE_LOG(LogTemp, Warning, TEXT("Started Behavior Tree for %s"), *ControlledVillager->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No Behavior Tree assigned to %s"), *GetName());
		}
	}
}
