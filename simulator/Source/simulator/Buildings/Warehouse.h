// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseBuilding.h"
#include "Warehouse.generated.h"

/**
 * Warehouse building for storing all types of resources
 * Large capacity general storage
 */
UCLASS()
class SIMULATOR_API AWarehouse : public ABaseBuilding
{
	GENERATED_BODY()

public:
	AWarehouse();

protected:
	virtual void BeginPlay() override;

public:
	// Get storage utilization percentage (0.0 to 1.0)
	UFUNCTION(BlueprintCallable, Category = "Warehouse")
	float GetStorageUtilization() const;

	// Check if warehouse is nearly full (>90%)
	UFUNCTION(BlueprintCallable, Category = "Warehouse")
	bool IsNearlyFull() const;

	// Get summary of stored resources
	UFUNCTION(BlueprintCallable, Category = "Warehouse")
	FString GetStorageSummary() const;
};
