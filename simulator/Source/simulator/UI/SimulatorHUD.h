// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SimulatorHUD.generated.h"

/**
 * Main HUD class for Simulator game
 * Manages all UI widgets and their lifecycle
 *
 * Usage in Blueprint (BP_SimulatorHUD):
 * 1. Create Blueprint inheriting from this class
 * 2. Set widget class references in Blueprint
 * 3. Set this as GameMode's HUD class
 * 4. Widgets will be automatically created on BeginPlay
 */
UCLASS()
class SIMULATOR_API ASimulatorHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASimulatorHUD();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// === Widget Management ===

	// Show/Hide resource display
	UFUNCTION(BlueprintCallable, Category = "Simulator HUD")
	void ShowResourceDisplay(bool bShow);

	// Show/Hide turn control
	UFUNCTION(BlueprintCallable, Category = "Simulator HUD")
	void ShowTurnControl(bool bShow);

	// Show/Hide building info
	UFUNCTION(BlueprintCallable, Category = "Simulator HUD")
	void ShowBuildingInfo(bool bShow);

	// Show/Hide villager list
	UFUNCTION(BlueprintCallable, Category = "Simulator HUD")
	void ShowVillagerList(bool bShow);

	// Show/Hide training queue
	UFUNCTION(BlueprintCallable, Category = "Simulator HUD")
	void ShowTrainingQueue(bool bShow);

	// Toggle all widgets
	UFUNCTION(BlueprintCallable, Category = "Simulator HUD")
	void ToggleAllWidgets();

	// === Widget Access ===

	UFUNCTION(BlueprintCallable, Category = "Simulator HUD")
	class UResourceDisplayWidget* GetResourceDisplayWidget() const { return ResourceDisplayWidget; }

	UFUNCTION(BlueprintCallable, Category = "Simulator HUD")
	class UTurnControlWidget* GetTurnControlWidget() const { return TurnControlWidget; }

	UFUNCTION(BlueprintCallable, Category = "Simulator HUD")
	class UBuildingInfoWidget* GetBuildingInfoWidget() const { return BuildingInfoWidget; }

	UFUNCTION(BlueprintCallable, Category = "Simulator HUD")
	class UVillagerListWidget* GetVillagerListWidget() const { return VillagerListWidget; }

	UFUNCTION(BlueprintCallable, Category = "Simulator HUD")
	class UTrainingQueueWidget* GetTrainingQueueWidget() const { return TrainingQueueWidget; }

	// === Initialization ===

	// Set territory for all widgets
	UFUNCTION(BlueprintCallable, Category = "Simulator HUD")
	void InitializeWithTerritory(class ATerritory* Territory);

protected:
	// Widget class references (set in Blueprint)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Simulator HUD|Widget Classes")
	TSubclassOf<class UResourceDisplayWidget> ResourceDisplayWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Simulator HUD|Widget Classes")
	TSubclassOf<class UTurnControlWidget> TurnControlWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Simulator HUD|Widget Classes")
	TSubclassOf<class UBuildingInfoWidget> BuildingInfoWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Simulator HUD|Widget Classes")
	TSubclassOf<class UVillagerListWidget> VillagerListWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Simulator HUD|Widget Classes")
	TSubclassOf<class UTrainingQueueWidget> TrainingQueueWidgetClass;

	// Widget instances
	UPROPERTY(BlueprintReadOnly, Category = "Simulator HUD|Widgets")
	class UResourceDisplayWidget* ResourceDisplayWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Simulator HUD|Widgets")
	class UTurnControlWidget* TurnControlWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Simulator HUD|Widgets")
	class UBuildingInfoWidget* BuildingInfoWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Simulator HUD|Widgets")
	class UVillagerListWidget* VillagerListWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Simulator HUD|Widgets")
	class UTrainingQueueWidget* TrainingQueueWidget;

	// Auto-update interval (seconds)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Simulator HUD|Settings")
	float UpdateInterval;

	// Timer for auto-update
	float UpdateTimer;

	// Are all widgets visible?
	bool bAllWidgetsVisible;

private:
	// Create all widgets
	void CreateWidgets();

	// Update all widgets
	void UpdateAllWidgets();
};
