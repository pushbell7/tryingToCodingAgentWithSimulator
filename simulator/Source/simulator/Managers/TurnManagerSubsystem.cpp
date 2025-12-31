// Copyright Epic Games, Inc. All Rights Reserved.

#include "TurnManagerSubsystem.h"
#include "BaseVillager.h"
#include "Territory.h"

void UTurnManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Villager action system
	MaxSimultaneousActions = 10;  // 10 actors can act per turn
	TurnDuration = 1.0f;          // Process requests every 1 second
	TurnTimer = 0.0f;

	// Territory turn system
	TerritoryTurnDuration = 60.0f;  // 60 seconds = 1 day
	TerritoryTurnTimer = 0.0f;
	CurrentTurn = 0;

	// Turn pause system
	bTurnPaused = false;
	bAutoPauseEnabled = true; // Default: pause before each turn
	bTurnReady = false;

	UE_LOG(LogTemp, Log, TEXT("TurnManagerSubsystem initialized"));
	UE_LOG(LogTemp, Log, TEXT("  Villager Actions: Max %d, Duration %.2f sec"),
		MaxSimultaneousActions, TurnDuration);
	UE_LOG(LogTemp, Log, TEXT("  Territory Turns: Duration %.0f sec (1 day)"),
		TerritoryTurnDuration);
	UE_LOG(LogTemp, Log, TEXT("  Auto-pause: %s"), bAutoPauseEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UTurnManagerSubsystem::Deinitialize()
{
	PendingRequests.Empty();
	ActiveActors.Empty();
	RegisteredTerritories.Empty();

	Super::Deinitialize();
}

void UTurnManagerSubsystem::Tick(float DeltaTime)
{
	// Process villager action requests every 1 second
	TurnTimer += DeltaTime;
	if (TurnTimer >= TurnDuration)
	{
		TurnTimer = 0.0f;
		ProcessActionRequests();
	}

	// Process territory turns every 60 seconds (1 day)
	if (!bTurnPaused)
	{
		TerritoryTurnTimer += DeltaTime;

		if (TerritoryTurnTimer >= TerritoryTurnDuration)
		{
			TerritoryTurnTimer = 0.0f;

			// Check if auto-pause is enabled
			if (bAutoPauseEnabled)
			{
				// Pause and wait for player input
				bTurnPaused = true;
				bTurnReady = true;

				UE_LOG(LogTemp, Warning, TEXT("======================================"));
				UE_LOG(LogTemp, Warning, TEXT("TURN PAUSED - Waiting for player input"));
				UE_LOG(LogTemp, Warning, TEXT("Call ResumeTurn() to continue"));
				UE_LOG(LogTemp, Warning, TEXT("======================================"));
			}
			else
			{
				// No pause - execute turn immediately
				ProcessTerritoryTurns();
			}
		}
	}
}

TStatId UTurnManagerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UTurnManagerSubsystem, STATGROUP_Tickables);
}

void UTurnManagerSubsystem::RequestAction(ABaseVillager* Actor, EActionType ActionType, ESocialClass SocialClass)
{
	if (!Actor)
		return;

	// Check if actor already has pending request
	for (const FActionRequest& Request : PendingRequests)
	{
		if (Request.RequestingActor == Actor)
		{
			UE_LOG(LogTemp, Verbose, TEXT("%s already has pending request"), *Actor->GetName());
			return;
		}
	}

	// Check if actor is already active
	if (ActiveActors.Contains(Actor))
	{
		UE_LOG(LogTemp, Verbose, TEXT("%s is already performing action"), *Actor->GetName());
		return;
	}

	// Create action request
	FActionRequest NewRequest;
	NewRequest.RequestingActor = Actor;
	NewRequest.ActionType = ActionType;
	NewRequest.SocialClass = SocialClass;
	NewRequest.Priority = CalculatePriority(SocialClass, ActionType);

	PendingRequests.Add(NewRequest);

	UE_LOG(LogTemp, Warning, TEXT("TurnManager REQUEST: %s - Action: %d, Priority: %.2f"),
		*Actor->GetName(), (int32)ActionType, NewRequest.Priority);
}

void UTurnManagerSubsystem::NotifyActionComplete(ABaseVillager* Actor)
{
	if (!Actor)
		return;

	ActiveActors.Remove(Actor);

	UE_LOG(LogTemp, Warning, TEXT("TurnManager COMPLETE: %s - Active actors: %d"),
		*Actor->GetName(), ActiveActors.Num());
}

void UTurnManagerSubsystem::ProcessActionRequests()
{
	if (PendingRequests.Num() == 0)
		return;

	UE_LOG(LogTemp, Warning, TEXT("TurnManager PROCESSING: %d requests - Active: %d/%d"),
		PendingRequests.Num(), ActiveActors.Num(), MaxSimultaneousActions);

	GrantActionPermissions();
}

void UTurnManagerSubsystem::GrantActionPermissions()
{
	// Sort requests by priority
	SortRequestsByPriority();

	int32 GrantedCount = 0;
	int32 AvailableSlots = MaxSimultaneousActions - ActiveActors.Num();

	// Grant permission to highest priority actors
	for (int32 i = PendingRequests.Num() - 1; i >= 0 && GrantedCount < AvailableSlots; i--)
	{
		FActionRequest& Request = PendingRequests[i];

		if (Request.RequestingActor)
		{
			// Grant permission - actor will start their action
			ActiveActors.Add(Request.RequestingActor);

			// Notify actor to start their action
			Request.RequestingActor->OnActionPermissionGranted(Request.ActionType);

			UE_LOG(LogTemp, Warning, TEXT("TurnManager GRANTED: %s - Action: %d, Priority: %.2f"),
				*Request.RequestingActor->GetName(), (int32)Request.ActionType, Request.Priority);

			GrantedCount++;
		}

		// Remove from pending queue
		PendingRequests.RemoveAt(i);
	}

	if (GrantedCount > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TurnManager SUMMARY: Granted %d actions - Active: %d/%d, Remaining: %d"),
			GrantedCount, ActiveActors.Num(), MaxSimultaneousActions, PendingRequests.Num());
	}
}

void UTurnManagerSubsystem::SortRequestsByPriority()
{
	PendingRequests.Sort([](const FActionRequest& A, const FActionRequest& B)
	{
		return A.Priority < B.Priority;  // Ascending (lowest first, we pop from back)
	});
}

float UTurnManagerSubsystem::CalculatePriority(ESocialClass SocialClass, EActionType ActionType)
{
	float BasePriority = 0.0f;

	// Social class weight (higher class = higher priority)
	switch (SocialClass)
	{
	case ESocialClass::Peasant:
		BasePriority = 1.0f;
		break;
	case ESocialClass::Commoner:
		BasePriority = 2.0f;
		break;
	case ESocialClass::Merchant:
		BasePriority = 3.0f;
		break;
	case ESocialClass::Soldier:
		BasePriority = 4.0f;
		break;
	case ESocialClass::Noble:
		BasePriority = 5.0f;
		break;
	case ESocialClass::Lord:
		BasePriority = 10.0f;
		break;
	}

	// Action type weight (combat > work > move)
	float ActionWeight = 1.0f;
	switch (ActionType)
	{
	case EActionType::Fight:
		ActionWeight = 3.0f;
		break;
	case EActionType::Work:
		ActionWeight = 2.0f;
		break;
	case EActionType::Trade:
		ActionWeight = 1.5f;
		break;
	case EActionType::Move:
		ActionWeight = 1.0f;
		break;
	case EActionType::Rest:
		ActionWeight = 0.5f;
		break;
	default:
		ActionWeight = 1.0f;
		break;
	}

	// Add random factor for variation
	float RandomFactor = FMath::RandRange(0.0f, 0.5f);

	return (BasePriority * ActionWeight) + RandomFactor;
}

// === Territory Turn System ===

void UTurnManagerSubsystem::RegisterTerritory(ATerritory* Territory)
{
	if (!Territory)
	{
		UE_LOG(LogTemp, Warning, TEXT("TurnManager: Cannot register null territory"));
		return;
	}

	if (RegisteredTerritories.Contains(Territory))
	{
		UE_LOG(LogTemp, Warning, TEXT("TurnManager: Territory %s already registered"), *Territory->TerritoryName);
		return;
	}

	RegisteredTerritories.Add(Territory);

	UE_LOG(LogTemp, Log, TEXT("TurnManager: Territory %s registered (Total: %d)"),
		*Territory->TerritoryName, RegisteredTerritories.Num());
}

void UTurnManagerSubsystem::UnregisterTerritory(ATerritory* Territory)
{
	if (!Territory)
		return;

	int32 Removed = RegisteredTerritories.Remove(Territory);

	if (Removed > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("TurnManager: Territory %s unregistered (Remaining: %d)"),
			*Territory->TerritoryName, RegisteredTerritories.Num());
	}
}

void UTurnManagerSubsystem::ProcessTerritoryTurns()
{
	if (RegisteredTerritories.Num() == 0)
		return;

	CurrentTurn++;

	UE_LOG(LogTemp, Warning, TEXT("======================================"));
	UE_LOG(LogTemp, Warning, TEXT("TURN %d BEGINNING - Processing %d territories"),
		CurrentTurn, RegisteredTerritories.Num());
	UE_LOG(LogTemp, Warning, TEXT("======================================"));

	// Process each territory's turn
	for (ATerritory* Territory : RegisteredTerritories)
	{
		if (Territory)
		{
			Territory->ProcessTurn();
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("======================================"));
	UE_LOG(LogTemp, Warning, TEXT("TURN %d COMPLETE"), CurrentTurn);
	UE_LOG(LogTemp, Warning, TEXT("======================================"));
}

// === Turn Pause System ===

void UTurnManagerSubsystem::ResumeTurn()
{
	if (!bTurnPaused)
	{
		UE_LOG(LogTemp, Warning, TEXT("TurnManager: Turn is not paused, cannot resume"));
		return;
	}

	if (!bTurnReady)
	{
		UE_LOG(LogTemp, Warning, TEXT("TurnManager: Turn is not ready yet, wait for timer"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("TurnManager: Resuming turn execution..."));

	// Unpause and execute the turn
	bTurnPaused = false;
	bTurnReady = false;

	ProcessTerritoryTurns();
}

void UTurnManagerSubsystem::SetAutoPause(bool bEnabled)
{
	bAutoPauseEnabled = bEnabled;

	UE_LOG(LogTemp, Log, TEXT("TurnManager: Auto-pause %s"),
		bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));

	// If disabling while paused, auto-resume
	if (!bEnabled && bTurnPaused && bTurnReady)
	{
		UE_LOG(LogTemp, Log, TEXT("TurnManager: Auto-resuming paused turn"));
		ResumeTurn();
	}
}
