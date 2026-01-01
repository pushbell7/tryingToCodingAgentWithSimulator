// Copyright Epic Games, Inc. All Rights Reserved.

#include "SimulatorHUD.h"
#include "ResourceDisplayWidget.h"
#include "TurnControlWidget.h"
#include "BuildingInfoWidget.h"
#include "VillagerListWidget.h"
#include "TrainingQueueWidget.h"
#include "Territory.h"
#include "Blueprint/UserWidget.h"

ASimulatorHUD::ASimulatorHUD()
{
	PrimaryActorTick.bCanEverTick = true;

	UpdateInterval = 1.0f; // Update every second
	UpdateTimer = 0.0f;
	bAllWidgetsVisible = true;
}

void ASimulatorHUD::BeginPlay()
{
	Super::BeginPlay();

	CreateWidgets();
}

void ASimulatorHUD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Auto-update widgets at interval
	UpdateTimer += DeltaTime;
	if (UpdateTimer >= UpdateInterval)
	{
		UpdateTimer = 0.0f;
		UpdateAllWidgets();
	}
}

void ASimulatorHUD::CreateWidgets()
{
	if (!GetWorld())
		return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
		return;

	// Create ResourceDisplayWidget
	if (ResourceDisplayWidgetClass)
	{
		ResourceDisplayWidget = CreateWidget<UResourceDisplayWidget>(PC, ResourceDisplayWidgetClass);
		if (ResourceDisplayWidget)
		{
			ResourceDisplayWidget->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("SimulatorHUD: Created ResourceDisplayWidget"));
		}
	}

	// Create TurnControlWidget
	if (TurnControlWidgetClass)
	{
		TurnControlWidget = CreateWidget<UTurnControlWidget>(PC, TurnControlWidgetClass);
		if (TurnControlWidget)
		{
			TurnControlWidget->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("SimulatorHUD: Created TurnControlWidget"));
		}
	}

	// Create BuildingInfoWidget
	if (BuildingInfoWidgetClass)
	{
		BuildingInfoWidget = CreateWidget<UBuildingInfoWidget>(PC, BuildingInfoWidgetClass);
		if (BuildingInfoWidget)
		{
			BuildingInfoWidget->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("SimulatorHUD: Created BuildingInfoWidget"));
		}
	}

	// Create VillagerListWidget
	if (VillagerListWidgetClass)
	{
		VillagerListWidget = CreateWidget<UVillagerListWidget>(PC, VillagerListWidgetClass);
		if (VillagerListWidget)
		{
			VillagerListWidget->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("SimulatorHUD: Created VillagerListWidget"));
		}
	}

	// Create TrainingQueueWidget
	if (TrainingQueueWidgetClass)
	{
		TrainingQueueWidget = CreateWidget<UTrainingQueueWidget>(PC, TrainingQueueWidgetClass);
		if (TrainingQueueWidget)
		{
			TrainingQueueWidget->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("SimulatorHUD: Created TrainingQueueWidget"));
		}
	}
}

void ASimulatorHUD::UpdateAllWidgets()
{
	if (ResourceDisplayWidget)
		ResourceDisplayWidget->UpdateResourceDisplay();

	if (TurnControlWidget)
		TurnControlWidget->UpdateTurnStatus();

	if (BuildingInfoWidget)
		BuildingInfoWidget->UpdateBuildingInfo();

	if (VillagerListWidget)
		VillagerListWidget->UpdateVillagerList();

	if (TrainingQueueWidget)
		TrainingQueueWidget->UpdateTrainingQueue();
}

void ASimulatorHUD::InitializeWithTerritory(ATerritory* Territory)
{
	if (!Territory)
	{
		UE_LOG(LogTemp, Warning, TEXT("SimulatorHUD: Invalid territory"));
		return;
	}

	if (ResourceDisplayWidget)
		ResourceDisplayWidget->SetTerritory(Territory);

	if (VillagerListWidget)
		VillagerListWidget->SetTerritory(Territory);

	UE_LOG(LogTemp, Log, TEXT("SimulatorHUD: Initialized with territory %s"), *Territory->TerritoryName);
}

void ASimulatorHUD::ShowResourceDisplay(bool bShow)
{
	if (ResourceDisplayWidget)
	{
		ResourceDisplayWidget->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void ASimulatorHUD::ShowTurnControl(bool bShow)
{
	if (TurnControlWidget)
	{
		TurnControlWidget->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void ASimulatorHUD::ShowBuildingInfo(bool bShow)
{
	if (BuildingInfoWidget)
	{
		BuildingInfoWidget->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void ASimulatorHUD::ShowVillagerList(bool bShow)
{
	if (VillagerListWidget)
	{
		VillagerListWidget->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void ASimulatorHUD::ShowTrainingQueue(bool bShow)
{
	if (TrainingQueueWidget)
	{
		TrainingQueueWidget->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void ASimulatorHUD::ToggleAllWidgets()
{
	bAllWidgetsVisible = !bAllWidgetsVisible;

	ShowResourceDisplay(bAllWidgetsVisible);
	ShowTurnControl(bAllWidgetsVisible);
	ShowBuildingInfo(bAllWidgetsVisible);
	ShowVillagerList(bAllWidgetsVisible);
	ShowTrainingQueue(bAllWidgetsVisible);

	UE_LOG(LogTemp, Log, TEXT("SimulatorHUD: Toggled all widgets to %s"),
		bAllWidgetsVisible ? TEXT("VISIBLE") : TEXT("HIDDEN"));
}
