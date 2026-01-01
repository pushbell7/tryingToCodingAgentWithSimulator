// Copyright Epic Games, Inc. All Rights Reserved.

#include "TurnControlWidget.h"
#include "TurnManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UTurnControlWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get TurnManager reference
	UWorld* World = GetWorld();
	if (World)
	{
		TurnManager = World->GetSubsystem<UTurnManagerSubsystem>();
	}

	// Initial update
	UpdateTurnStatus();
}

void UTurnControlWidget::UpdateTurnStatus()
{
	if (!TurnManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("TurnControlWidget: No TurnManager found"));
		return;
	}

	// Cache current state
	bool bPreviousPaused = bIsPaused;
	bool bPreviousAutoPause = bAutoPauseEnabled;
	int32 PreviousTurn = CurrentTurn;

	bIsPaused = TurnManager->IsTurnPaused();
	bAutoPauseEnabled = TurnManager->IsAutoPauseEnabled();
	CurrentTurn = TurnManager->GetCurrentTurn();
	TimeUntilNextTurn = 60.0f; // TODO: Get actual timer from TurnManager

	// Notify Blueprint if state changed
	if (bPreviousPaused != bIsPaused ||
		bPreviousAutoPause != bAutoPauseEnabled ||
		PreviousTurn != CurrentTurn)
	{
		OnTurnStateChanged(bIsPaused, bAutoPauseEnabled, CurrentTurn);
	}
}

void UTurnControlWidget::ResumeTurn()
{
	if (!TurnManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("TurnControlWidget: Cannot resume - no TurnManager"));
		return;
	}

	TurnManager->ResumeTurn();
	UpdateTurnStatus();

	UE_LOG(LogTemp, Log, TEXT("TurnControlWidget: Turn resumed by player"));
}

void UTurnControlWidget::ToggleAutoPause()
{
	if (!TurnManager)
		return;

	bool bNewState = !TurnManager->IsAutoPauseEnabled();
	TurnManager->SetAutoPause(bNewState);
	UpdateTurnStatus();

	UE_LOG(LogTemp, Log, TEXT("TurnControlWidget: Auto-pause toggled to %s"),
		bNewState ? TEXT("ON") : TEXT("OFF"));
}

void UTurnControlWidget::SetAutoPause(bool bEnabled)
{
	if (!TurnManager)
		return;

	TurnManager->SetAutoPause(bEnabled);
	UpdateTurnStatus();
}

int32 UTurnControlWidget::GetCurrentTurn() const
{
	return CurrentTurn;
}

bool UTurnControlWidget::IsTurnPaused() const
{
	return bIsPaused;
}

bool UTurnControlWidget::IsAutoPauseEnabled() const
{
	return bAutoPauseEnabled;
}

float UTurnControlWidget::GetTimeUntilNextTurn() const
{
	return TimeUntilNextTurn;
}

float UTurnControlWidget::GetTurnProgressPercent() const
{
	// 60 seconds per turn
	float TurnDuration = 60.0f;
	float Elapsed = TurnDuration - TimeUntilNextTurn;
	return FMath::Clamp(Elapsed / TurnDuration, 0.0f, 1.0f);
}
