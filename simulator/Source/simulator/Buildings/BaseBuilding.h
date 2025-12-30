// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimulatorTypes.h"
#include "BaseBuilding.generated.h"

/**
 * Base class for all buildings in settlement zones
 * Buildings can store resources, process them, or provide special functions
 */
UCLASS()
class SIMULATOR_API ABaseBuilding : public AActor
{
	GENERATED_BODY()

public:
	ABaseBuilding();

protected:
	virtual void BeginPlay() override;

public:
	// Building type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	EBuildingType BuildingType;

	// Display name for this building
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	FString BuildingName;

	// Visual mesh component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
	class UStaticMeshComponent* BuildingMesh;

	// Inventory component for storage buildings
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
	class UInventoryComponent* Inventory;

	// Is this building operational (has workers, not damaged, etc)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	bool bIsOperational;

	// Maximum number of workers that can work here
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 MaxWorkers;

	// Current number of workers assigned
	UPROPERTY(BlueprintReadOnly, Category = "Building")
	int32 CurrentWorkers;

	// Construction cost for this building type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Construction")
	FConstructionCost ConstructionCost;

	// === Production System ===

	// Production recipe for this building (what it produces)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Production")
	FCraftingRecipe ProductionRecipe;

	// Optimal number of workers for 100% efficiency
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Production")
	int32 OptimalWorkerCount;

	// Can this building produce resources?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Production")
	bool bCanProduce;

	// Calculate production output based on current workers
	UFUNCTION(BlueprintCallable, Category = "Building|Production")
	TMap<EResourceType, int32> CalculateProduction();

	// Calculate labor efficiency (0.0 - 1.0+)
	UFUNCTION(BlueprintCallable, Category = "Building|Production")
	float CalculateLaborEfficiency() const;

	// Check if building can accept resources
	UFUNCTION(BlueprintCallable, Category = "Building")
	virtual bool CanAcceptResources() const;

	// Check if building has resources available
	UFUNCTION(BlueprintCallable, Category = "Building")
	virtual bool HasResources() const;

	// Get building location for AI navigation
	UFUNCTION(BlueprintCallable, Category = "Building")
	FVector GetBuildingLocation() const;

	// Check if this is a storage building
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool IsStorageBuilding() const;

	// Check if this is a processing building
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool IsProcessingBuilding() const;

	// Assign a worker to this building
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool AddWorker(class ABaseVillager* Worker);

	// Remove a worker from this building
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool RemoveWorker(class ABaseVillager* Worker);

	// Check if building has available worker slots
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool HasAvailableWorkerSlots() const;

protected:
	// Workers currently assigned to this building
	UPROPERTY()
	TArray<class ABaseVillager*> AssignedWorkers;
};
