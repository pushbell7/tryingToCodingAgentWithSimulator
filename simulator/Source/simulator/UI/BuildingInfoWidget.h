// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SimulatorTypes.h"
#include "BuildingInfoWidget.generated.h"

/**
 * Building information struct for UI display
 */
USTRUCT(BlueprintType)
struct FBuildingDisplayInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Building")
	FString BuildingName;

	UPROPERTY(BlueprintReadOnly, Category = "Building")
	EBuildingType BuildingType;

	UPROPERTY(BlueprintReadOnly, Category = "Building")
	bool bIsOperational;

	UPROPERTY(BlueprintReadOnly, Category = "Building")
	bool bCanProduce;

	UPROPERTY(BlueprintReadOnly, Category = "Building")
	int32 CurrentWorkers;

	UPROPERTY(BlueprintReadOnly, Category = "Building")
	int32 MaxWorkers;

	UPROPERTY(BlueprintReadOnly, Category = "Building")
	int32 OptimalWorkers;

	UPROPERTY(BlueprintReadOnly, Category = "Building")
	float LaborEfficiency;

	UPROPERTY(BlueprintReadOnly, Category = "Building")
	ESkillLevel RequiredSkillLevel;

	UPROPERTY(BlueprintReadOnly, Category = "Building")
	bool bHasInputResources;

	UPROPERTY(BlueprintReadOnly, Category = "Building")
	TArray<FResourceStack> InputResources;

	UPROPERTY(BlueprintReadOnly, Category = "Building")
	TArray<FResourceStack> OutputResources;

	FBuildingDisplayInfo()
		: BuildingName(TEXT(""))
		, BuildingType(EBuildingType::Warehouse)
		, bIsOperational(false)
		, bCanProduce(false)
		, CurrentWorkers(0)
		, MaxWorkers(0)
		, OptimalWorkers(0)
		, LaborEfficiency(0.0f)
		, RequiredSkillLevel(ESkillLevel::Novice)
		, bHasInputResources(true)
	{}
};

/**
 * Widget for displaying building production status
 * Base C++ class - inherit in Blueprint to design visual layout
 *
 * Usage in Blueprint (WBP_BuildingInfo):
 * 1. Create Widget Blueprint inheriting from this class
 * 2. Add Text/ProgressBar widgets for workers, efficiency, resources
 * 3. Call SetBuilding() to bind to a building
 * 4. Call UpdateBuildingInfo() to refresh data
 * 5. Implement OnBuildingInfoUpdated event to update visual widgets
 */
UCLASS()
class SIMULATOR_API UBuildingInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Set building to display
	UFUNCTION(BlueprintCallable, Category = "Building Info")
	void SetBuilding(class ABaseBuilding* InBuilding);

	// Update building information
	UFUNCTION(BlueprintCallable, Category = "Building Info")
	void UpdateBuildingInfo();

	// Get building display info
	UFUNCTION(BlueprintCallable, Category = "Building Info")
	FBuildingDisplayInfo GetBuildingInfo() const;

	// Get worker count text (e.g., "2/4 (Optimal: 3)")
	UFUNCTION(BlueprintCallable, Category = "Building Info")
	FString GetWorkerStatusText() const;

	// Get efficiency percentage (0-100)
	UFUNCTION(BlueprintCallable, Category = "Building Info")
	int32 GetEfficiencyPercent() const;

	// Get production status text
	UFUNCTION(BlueprintCallable, Category = "Building Info")
	FString GetProductionStatusText() const;

protected:
	// Called when building info is updated
	UFUNCTION(BlueprintImplementableEvent, Category = "Building Info")
	void OnBuildingInfoUpdated();

	// Target building
	UPROPERTY(BlueprintReadOnly, Category = "Building Info")
	class ABaseBuilding* TargetBuilding;

	// Cached building info
	UPROPERTY(BlueprintReadOnly, Category = "Building Info")
	FBuildingDisplayInfo CachedInfo;
};
