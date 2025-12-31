// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseBuilding.h"
#include "SimulatorTypes.h"
#include "GuildHall.generated.h"

/**
 * Training request for skill improvement
 */
USTRUCT(BlueprintType)
struct FTrainingRequest
{
	GENERATED_BODY()

	// Villager being trained
	UPROPERTY()
	class ABaseVillager* Trainee;

	// Building type to learn (profession)
	UPROPERTY()
	EBuildingType Profession;

	// Target skill level
	UPROPERTY()
	ESkillLevel TargetLevel;

	// Training progress (0.0 - 1.0)
	UPROPERTY()
	float Progress;

	// Training duration in turns (total)
	UPROPERTY()
	int32 TotalTurns;

	// Turns completed
	UPROPERTY()
	int32 TurnsCompleted;

	FTrainingRequest()
		: Trainee(nullptr)
		, Profession(EBuildingType::Warehouse)
		, TargetLevel(ESkillLevel::Novice)
		, Progress(0.0f)
		, TotalTurns(0)
		, TurnsCompleted(0)
	{}
};

/**
 * Guild Hall - Medieval skill training building
 * Trains villagers to improve their skills for specific professions
 * Based on historical guild apprenticeship system
 */
UCLASS()
class SIMULATOR_API AGuildHall : public ABaseBuilding
{
	GENERATED_BODY()

public:
	AGuildHall();

protected:
	virtual void BeginPlay() override;

public:
	// === Training System ===

	// Current training request (one at a time)
	UPROPERTY(BlueprintReadOnly, Category = "Guild Hall|Training")
	FTrainingRequest CurrentTraining;

	// Queue of pending training requests
	UPROPERTY(BlueprintReadOnly, Category = "Guild Hall|Training")
	TArray<FTrainingRequest> TrainingQueue;

	// Is someone currently being trained?
	UPROPERTY(BlueprintReadOnly, Category = "Guild Hall|Training")
	bool bIsTraining;

	// Start training a villager for a specific profession
	UFUNCTION(BlueprintCallable, Category = "Guild Hall|Training")
	bool StartTraining(class ABaseVillager* Villager, EBuildingType Profession, ESkillLevel TargetLevel);

	// Process one turn of training
	UFUNCTION(BlueprintCallable, Category = "Guild Hall|Training")
	void ProcessTrainingTurn();

	// Complete current training
	UFUNCTION(BlueprintCallable, Category = "Guild Hall|Training")
	void CompleteTraining();

	// Cancel current training
	UFUNCTION(BlueprintCallable, Category = "Guild Hall|Training")
	void CancelTraining();

	// Get training cost for a skill upgrade
	UFUNCTION(BlueprintCallable, Category = "Guild Hall|Training")
	TMap<EResourceType, int32> GetTrainingCost(ESkillLevel CurrentLevel, ESkillLevel TargetLevel) const;

	// Get training duration for a skill upgrade (in turns)
	UFUNCTION(BlueprintCallable, Category = "Guild Hall|Training")
	int32 GetTrainingDuration(ESkillLevel CurrentLevel, ESkillLevel TargetLevel) const;

	// Check if can afford training
	UFUNCTION(BlueprintCallable, Category = "Guild Hall|Training")
	bool CanAffordTraining(ESkillLevel CurrentLevel, ESkillLevel TargetLevel) const;

	// === Training Costs ===

	// Base cost multiplier per skill level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild Hall|Costs")
	float CostMultiplier;

	// Base training duration in turns
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild Hall|Costs")
	int32 BaseDurationTurns;
};
