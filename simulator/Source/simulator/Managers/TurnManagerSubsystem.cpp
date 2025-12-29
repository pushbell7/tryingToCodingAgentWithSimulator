// Copyright Epic Games, Inc. All Rights Reserved.

#include "TurnManagerSubsystem.h"
#include "BaseVillager.h"

void UTurnManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MaxSimultaneousActions = 10;  // 10 actors can act per turn
	TurnDuration = 1.0f;          // Process requests every 1 second
	TurnTimer = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("TurnManagerSubsystem initialized - Max Actions: %d, Turn Duration: %.2f"),
		MaxSimultaneousActions, TurnDuration);
}

void UTurnManagerSubsystem::Deinitialize()
{
	PendingRequests.Empty();
	ActiveActors.Empty();

	Super::Deinitialize();
}

void UTurnManagerSubsystem::Tick(float DeltaTime)
{
	TurnTimer += DeltaTime;

	// Process action requests every turn
	if (TurnTimer >= TurnDuration)
	{
		TurnTimer = 0.0f;
		ProcessActionRequests();
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

	UE_LOG(LogTemp, Verbose, TEXT("%s requested action (Type: %d, Priority: %.2f)"),
		*Actor->GetName(), (int32)ActionType, NewRequest.Priority);
}

void UTurnManagerSubsystem::NotifyActionComplete(ABaseVillager* Actor)
{
	if (!Actor)
		return;

	ActiveActors.Remove(Actor);

	UE_LOG(LogTemp, Verbose, TEXT("%s completed action - Active actors: %d"),
		*Actor->GetName(), ActiveActors.Num());
}

void UTurnManagerSubsystem::ProcessActionRequests()
{
	if (PendingRequests.Num() == 0)
		return;

	UE_LOG(LogTemp, Verbose, TEXT("Processing %d action requests - Active: %d/%d"),
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

			// Notify actor (will be implemented in BaseVillager)
			// Request.RequestingActor->OnActionPermissionGranted(Request.ActionType);

			UE_LOG(LogTemp, Verbose, TEXT("GRANTED: %s - Action: %d, Priority: %.2f"),
				*Request.RequestingActor->GetName(), (int32)Request.ActionType, Request.Priority);

			GrantedCount++;
		}

		// Remove from pending queue
		PendingRequests.RemoveAt(i);
	}

	UE_LOG(LogTemp, Verbose, TEXT("Granted %d actions - Active: %d/%d, Remaining requests: %d"),
		GrantedCount, ActiveActors.Num(), MaxSimultaneousActions, PendingRequests.Num());
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
