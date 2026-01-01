// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TurnControlWidget.generated.h"

/**
 * Widget for controlling turn system (pause/resume)
 * Base C++ class - inherit in Blueprint to design visual layout
 *
 * Usage in Blueprint (WBP_TurnControl):
 * 1. Create Widget Blueprint inheriting from this class
 * 2. Add Button widgets for Pause/Resume/Toggle Auto-Pause
 * 3. Bind button OnClicked events to BP functions that call these C++ functions
 * 4. Implement OnTurnStateChanged event to update button states
 * 5. Call UpdateTurnStatus() regularly (e.g., on Tick) to refresh display
 */
UCLASS()
class SIMULATOR_API UTurnControlWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Initialize widget
	virtual void NativeConstruct() override;

	// Update turn status display
	UFUNCTION(BlueprintCallable, Category = "Turn Control")
	void UpdateTurnStatus();

	// Resume turn execution (player finished making decisions)
	UFUNCTION(BlueprintCallable, Category = "Turn Control")
	void ResumeTurn();

	// Toggle auto-pause on/off
	UFUNCTION(BlueprintCallable, Category = "Turn Control")
	void ToggleAutoPause();

	// Set auto-pause enabled/disabled
	UFUNCTION(BlueprintCallable, Category = "Turn Control")
	void SetAutoPause(bool bEnabled);

	// Get current turn number
	UFUNCTION(BlueprintCallable, Category = "Turn Control")
	int32 GetCurrentTurn() const;

	// Is turn currently paused?
	UFUNCTION(BlueprintCallable, Category = "Turn Control")
	bool IsTurnPaused() const;

	// Is auto-pause enabled?
	UFUNCTION(BlueprintCallable, Category = "Turn Control")
	bool IsAutoPauseEnabled() const;

	// Get time until next turn (seconds)
	UFUNCTION(BlueprintCallable, Category = "Turn Control")
	float GetTimeUntilNextTurn() const;

	// Get turn progress percentage (0.0 - 1.0)
	UFUNCTION(BlueprintCallable, Category = "Turn Control")
	float GetTurnProgressPercent() const;

protected:
	// Called when turn state changes (paused/resumed/turn executed)
	UFUNCTION(BlueprintImplementableEvent, Category = "Turn Control")
	void OnTurnStateChanged(bool bPaused, bool bAutoPause, int32 TurnNumber);

	// Cached turn manager reference
	UPROPERTY(BlueprintReadOnly, Category = "Turn Control")
	class UTurnManagerSubsystem* TurnManager;

	// Cached state
	UPROPERTY(BlueprintReadOnly, Category = "Turn Control")
	bool bIsPaused;

	UPROPERTY(BlueprintReadOnly, Category = "Turn Control")
	bool bAutoPauseEnabled;

	UPROPERTY(BlueprintReadOnly, Category = "Turn Control")
	int32 CurrentTurn;

	UPROPERTY(BlueprintReadOnly, Category = "Turn Control")
	float TimeUntilNextTurn;
};
