// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SimulatorTypes.h"
#include "TrainingQueueWidget.generated.h"

/**
 * Training request display info
 */
USTRUCT(BlueprintType)
struct FTrainingDisplayInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Training")
	FString VillagerName;

	UPROPERTY(BlueprintReadOnly, Category = "Training")
	EBuildingType Profession;

	UPROPERTY(BlueprintReadOnly, Category = "Training")
	FString ProfessionName;

	UPROPERTY(BlueprintReadOnly, Category = "Training")
	ESkillLevel CurrentLevel;

	UPROPERTY(BlueprintReadOnly, Category = "Training")
	ESkillLevel TargetLevel;

	UPROPERTY(BlueprintReadOnly, Category = "Training")
	float Progress;

	UPROPERTY(BlueprintReadOnly, Category = "Training")
	int32 TurnsCompleted;

	UPROPERTY(BlueprintReadOnly, Category = "Training")
	int32 TotalTurns;

	UPROPERTY(BlueprintReadOnly, Category = "Training")
	bool bIsActive;

	FTrainingDisplayInfo()
		: VillagerName(TEXT(""))
		, Profession(EBuildingType::Warehouse)
		, ProfessionName(TEXT(""))
		, CurrentLevel(ESkillLevel::Novice)
		, TargetLevel(ESkillLevel::Apprentice)
		, Progress(0.0f)
		, TurnsCompleted(0)
		, TotalTurns(0)
		, bIsActive(false)
	{}
};

/**
 * Widget for displaying GuildHall training queue
 * Base C++ class - inherit in Blueprint to design visual layout
 *
 * Usage in Blueprint (WBP_TrainingQueue):
 * 1. Create Widget Blueprint inheriting from this class
 * 2. Add ListView for training queue
 * 3. Add Button to start new training
 * 4. Call SetGuildHall() to bind to a guild hall
 * 5. Call UpdateTrainingQueue() to refresh data
 * 6. Implement OnTrainingQueueUpdated event
 */
UCLASS()
class SIMULATOR_API UTrainingQueueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Set guild hall to display
	UFUNCTION(BlueprintCallable, Category = "Training Queue")
	void SetGuildHall(class AGuildHall* InGuildHall);

	// Update training queue display
	UFUNCTION(BlueprintCallable, Category = "Training Queue")
	void UpdateTrainingQueue();

	// Get all training requests (active + queued)
	UFUNCTION(BlueprintCallable, Category = "Training Queue")
	TArray<FTrainingDisplayInfo> GetTrainingQueue() const;

	// Get current active training
	UFUNCTION(BlueprintCallable, Category = "Training Queue")
	FTrainingDisplayInfo GetActiveTraining() const;

	// Start new training
	UFUNCTION(BlueprintCallable, Category = "Training Queue")
	bool StartTraining(class ABaseVillager* Villager, EBuildingType Profession, ESkillLevel TargetLevel);

	// Cancel current training
	UFUNCTION(BlueprintCallable, Category = "Training Queue")
	void CancelTraining();

	// Get training cost for skill upgrade
	UFUNCTION(BlueprintCallable, Category = "Training Queue")
	TMap<EResourceType, int32> GetTrainingCost(ESkillLevel CurrentLevel, ESkillLevel TargetLevel) const;

	// Get training duration for skill upgrade
	UFUNCTION(BlueprintCallable, Category = "Training Queue")
	int32 GetTrainingDuration(ESkillLevel CurrentLevel, ESkillLevel TargetLevel) const;

	// Check if can afford training
	UFUNCTION(BlueprintCallable, Category = "Training Queue")
	bool CanAffordTraining(ESkillLevel CurrentLevel, ESkillLevel TargetLevel) const;

	// Get training cost text (e.g., "50 Food, 10 Tools")
	UFUNCTION(BlueprintCallable, Category = "Training Queue")
	FString GetTrainingCostText(ESkillLevel CurrentLevel, ESkillLevel TargetLevel) const;

protected:
	// Called when training queue is updated
	UFUNCTION(BlueprintImplementableEvent, Category = "Training Queue")
	void OnTrainingQueueUpdated();

	// Target guild hall
	UPROPERTY(BlueprintReadOnly, Category = "Training Queue")
	class AGuildHall* TargetGuildHall;

	// Cached training queue
	UPROPERTY(BlueprintReadOnly, Category = "Training Queue")
	TArray<FTrainingDisplayInfo> CachedQueue;

	// Is training active?
	UPROPERTY(BlueprintReadOnly, Category = "Training Queue")
	bool bIsTraining;

	// Queue size
	UPROPERTY(BlueprintReadOnly, Category = "Training Queue")
	int32 QueueSize;
};
