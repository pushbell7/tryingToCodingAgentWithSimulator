// Copyright Epic Games, Inc. All Rights Reserved.

#include "BuildingInfoWidget.h"
#include "BaseBuilding.h"

void UBuildingInfoWidget::SetBuilding(ABaseBuilding* InBuilding)
{
	TargetBuilding = InBuilding;
	UpdateBuildingInfo();
}

void UBuildingInfoWidget::UpdateBuildingInfo()
{
	if (!TargetBuilding)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildingInfoWidget: No building set"));
		return;
	}

	// Populate display info
	CachedInfo.BuildingName = TargetBuilding->BuildingName;
	CachedInfo.BuildingType = TargetBuilding->BuildingType;
	CachedInfo.bIsOperational = TargetBuilding->bIsOperational;
	CachedInfo.bCanProduce = TargetBuilding->bCanProduce;
	CachedInfo.CurrentWorkers = TargetBuilding->CurrentWorkers;
	CachedInfo.MaxWorkers = TargetBuilding->MaxWorkers;
	CachedInfo.OptimalWorkers = TargetBuilding->OptimalWorkerCount;
	CachedInfo.LaborEfficiency = TargetBuilding->CalculateLaborEfficiency();
	CachedInfo.RequiredSkillLevel = TargetBuilding->RequiredSkillLevel;
	CachedInfo.bHasInputResources = TargetBuilding->HasInputResources();
	CachedInfo.InputResources = TargetBuilding->ProductionRecipe.InputResources;
	CachedInfo.OutputResources = TargetBuilding->ProductionRecipe.OutputResources;

	// Notify Blueprint
	OnBuildingInfoUpdated();
}

FBuildingDisplayInfo UBuildingInfoWidget::GetBuildingInfo() const
{
	return CachedInfo;
}

FString UBuildingInfoWidget::GetWorkerStatusText() const
{
	if (!TargetBuilding)
		return TEXT("No Building");

	return FString::Printf(TEXT("%d/%d (Optimal: %d)"),
		CachedInfo.CurrentWorkers,
		CachedInfo.MaxWorkers,
		CachedInfo.OptimalWorkers);
}

int32 UBuildingInfoWidget::GetEfficiencyPercent() const
{
	return FMath::RoundToInt(CachedInfo.LaborEfficiency * 100.0f);
}

FString UBuildingInfoWidget::GetProductionStatusText() const
{
	if (!TargetBuilding)
		return TEXT("No Building");

	if (!CachedInfo.bCanProduce)
		return TEXT("Not a Production Building");

	if (!CachedInfo.bIsOperational)
		return TEXT("Not Operational");

	if (CachedInfo.CurrentWorkers == 0)
		return TEXT("No Workers");

	if (!CachedInfo.bHasInputResources && CachedInfo.InputResources.Num() > 0)
		return TEXT("Insufficient Resources");

	return FString::Printf(TEXT("Producing (%d%% efficiency)"),
		GetEfficiencyPercent());
}
