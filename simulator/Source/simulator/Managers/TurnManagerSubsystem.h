// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SimulatorTypes.h"
#include "TurnManagerSubsystem.generated.h"

/**
 * Action request from an actor
 */
USTRUCT(BlueprintType)
struct FActionRequest
{
	GENERATED_BODY()

	UPROPERTY()
	class ABaseVillager* RequestingActor;

	UPROPERTY()
	EActionType ActionType;

	UPROPERTY()
	ESocialClass SocialClass;

	UPROPERTY()
	float Priority;

	FActionRequest()
		: RequestingActor(nullptr)
		, ActionType(EActionType::None)
		, SocialClass(ESocialClass::Peasant)
		, Priority(0.0f)
	{}
};

/**
 * Manages turn-based action system as a WorldSubsystem
 * Actors request actions, manager grants permission based on priority
 */
UCLASS()
class SIMULATOR_API UTurnManagerSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// UTickableWorldSubsystem implementation
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	// Request action permission
	UFUNCTION(BlueprintCallable, Category = "Turn Manager")
	void RequestAction(class ABaseVillager* Actor, EActionType ActionType, ESocialClass SocialClass);

	// Notify action completion
	UFUNCTION(BlueprintCallable, Category = "Turn Manager")
	void NotifyActionComplete(class ABaseVillager* Actor);

	// Get current active actor count
	UFUNCTION(BlueprintCallable, Category = "Turn Manager")
	int32 GetActiveActorCount() const { return ActiveActors.Num(); }

	// Get pending request count
	UFUNCTION(BlueprintCallable, Category = "Turn Manager")
	int32 GetPendingRequestCount() const { return PendingRequests.Num(); }

	// === Territory Turn System ===

	// Register a territory for turn processing
	UFUNCTION(BlueprintCallable, Category = "Turn Manager|Territory")
	void RegisterTerritory(class ATerritory* Territory);

	// Unregister a territory
	UFUNCTION(BlueprintCallable, Category = "Turn Manager|Territory")
	void UnregisterTerritory(class ATerritory* Territory);

	// Get current turn number
	UFUNCTION(BlueprintCallable, Category = "Turn Manager|Territory")
	int32 GetCurrentTurn() const { return CurrentTurn; }

	// Get registered territory count
	UFUNCTION(BlueprintCallable, Category = "Turn Manager|Territory")
	int32 GetTerritoryCount() const { return RegisteredTerritories.Num(); }

	// === Turn Pause System ===

	// Is the turn system paused (waiting for player input)?
	UFUNCTION(BlueprintCallable, Category = "Turn Manager|Pause")
	bool IsTurnPaused() const { return bTurnPaused; }

	// Resume turn execution (player finished making decisions)
	UFUNCTION(BlueprintCallable, Category = "Turn Manager|Pause")
	void ResumeTurn();

	// Enable/disable automatic turn pause
	UFUNCTION(BlueprintCallable, Category = "Turn Manager|Pause")
	void SetAutoPause(bool bEnabled);

	// Is auto-pause enabled?
	UFUNCTION(BlueprintCallable, Category = "Turn Manager|Pause")
	bool IsAutoPauseEnabled() const { return bAutoPauseEnabled; }

protected:
	// Process pending action requests
	void ProcessActionRequests();

	// Process territory turns (production/consumption)
	void ProcessTerritoryTurns();

	// Grant action permission to highest priority actors
	void GrantActionPermissions();

	// Sort requests by priority
	void SortRequestsByPriority();

	// Calculate priority for an action request
	float CalculatePriority(ESocialClass SocialClass, EActionType ActionType);

private:
	// === Villager Action System ===

	// Pending action requests (queue)
	UPROPERTY()
	TArray<FActionRequest> PendingRequests;

	// Currently active actors
	UPROPERTY()
	TArray<class ABaseVillager*> ActiveActors;

	// Maximum number of actors that can act simultaneously
	int32 MaxSimultaneousActions;

	// Turn duration (how often to process action requests)
	float TurnDuration;

	// Timer for turn processing
	float TurnTimer;

	// === Territory Turn System ===

	// Registered territories for turn processing
	UPROPERTY()
	TArray<class ATerritory*> RegisteredTerritories;

	// Territory turn duration (1 minute = 1 day)
	float TerritoryTurnDuration;

	// Timer for territory turns
	float TerritoryTurnTimer;

	// Current turn number
	int32 CurrentTurn;

	// === Turn Pause System ===

	// Is turn execution paused (waiting for player)?
	bool bTurnPaused;

	// Should automatically pause before each turn?
	bool bAutoPauseEnabled;

	// Turn is ready to execute (timer expired, waiting for resume)
	bool bTurnReady;
};
