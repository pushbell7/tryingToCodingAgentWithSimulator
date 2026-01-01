// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SimulatorTypes.h"
#include "ResourceDisplayWidget.generated.h"

/**
 * Resource information struct for UI display
 */
USTRUCT(BlueprintType)
struct FResourceDisplayInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Resource")
	EResourceType ResourceType;

	UPROPERTY(BlueprintReadOnly, Category = "Resource")
	int32 CurrentAmount;

	UPROPERTY(BlueprintReadOnly, Category = "Resource")
	int32 MaxCapacity;

	UPROPERTY(BlueprintReadOnly, Category = "Resource")
	FString ResourceName;

	FResourceDisplayInfo()
		: ResourceType(EResourceType::Food)
		, CurrentAmount(0)
		, MaxCapacity(0)
		, ResourceName(TEXT(""))
	{}
};

/**
 * Widget for displaying Territory resource dashboard
 * Base C++ class - inherit in Blueprint to design visual layout
 *
 * Usage in Blueprint (WBP_ResourceDisplay):
 * 1. Create Widget Blueprint inheriting from this class
 * 2. Add Text/Image/ProgressBar widgets for each resource
 * 3. Call UpdateResourceDisplay() to refresh data
 * 4. Implement OnResourcesUpdated event to update visual widgets
 */
UCLASS()
class SIMULATOR_API UResourceDisplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Initialize widget with territory
	UFUNCTION(BlueprintCallable, Category = "Resource Display")
	void SetTerritory(class ATerritory* InTerritory);

	// Update resource display from territory
	UFUNCTION(BlueprintCallable, Category = "Resource Display")
	void UpdateResourceDisplay();

	// Get all resources as array for easy iteration in Blueprint
	UFUNCTION(BlueprintCallable, Category = "Resource Display")
	TArray<FResourceDisplayInfo> GetResourceList() const;

	// Get specific resource amount
	UFUNCTION(BlueprintCallable, Category = "Resource Display")
	int32 GetResourceAmount(EResourceType ResourceType) const;

	// Get storage capacity percentage (0.0 - 1.0)
	UFUNCTION(BlueprintCallable, Category = "Resource Display")
	float GetStorageUsagePercent() const;

protected:
	// Called when resources are updated - implement in Blueprint to update UI
	UFUNCTION(BlueprintImplementableEvent, Category = "Resource Display")
	void OnResourcesUpdated();

	// Territory to display resources from
	UPROPERTY(BlueprintReadOnly, Category = "Resource Display")
	class ATerritory* TargetTerritory;

	// Cached resource data
	UPROPERTY(BlueprintReadOnly, Category = "Resource Display")
	TArray<FResourceDisplayInfo> CachedResources;

	// Total resource count
	UPROPERTY(BlueprintReadOnly, Category = "Resource Display")
	int32 TotalResourceAmount;

	// Max storage capacity
	UPROPERTY(BlueprintReadOnly, Category = "Resource Display")
	int32 MaxStorageCapacity;
};
