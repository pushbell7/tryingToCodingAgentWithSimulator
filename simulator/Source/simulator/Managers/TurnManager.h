// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimulatorTypes.h"
#include "TurnManager.generated.h"

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
 * Manages turn-based action system
 * Actors request actions, manager grants permission based on priority
 */
UCLASS()
class SIMULATOR_API ATurnManager : public AActor
{
	GENERATED_BODY()

public:
	ATurnManager();

	virtual void Tick(float DeltaTime) override;

	// Request action permission
	UFUNCTION(BlueprintCallable, Category = "Turn Manager")
	void RequestAction(class ABaseVillager* Actor, EActionType ActionType, ESocialClass SocialClass);

	// Notify action completion
	UFUNCTION(BlueprintCallable, Category = "Turn Manager")
	void NotifyActionComplete(class ABaseVillager* Actor);

protected:
	virtual void BeginPlay() override;

	// Process pending action requests
	void ProcessActionRequests();

	// Grant action permission to highest priority actors
	void GrantActionPermissions();

private:
	// Pending action requests (queue)
	UPROPERTY()
	TArray<FActionRequest> PendingRequests;

	// Currently active actors
	UPROPERTY()
	TArray<class ABaseVillager*> ActiveActors;

	// Maximum number of actors that can act simultaneously
	UPROPERTY(EditAnywhere, Category = "Turn Manager")
	int32 MaxSimultaneousActions;

	// Turn duration (how often to process requests)
	UPROPERTY(EditAnywhere, Category = "Turn Manager")
	float TurnDuration;

	// Timer for turn processing
	float TurnTimer;

	// Sort requests by priority
	void SortRequestsByPriority();

	// Calculate priority for an action request
	float CalculatePriority(ESocialClass SocialClass, EActionType ActionType);
};
