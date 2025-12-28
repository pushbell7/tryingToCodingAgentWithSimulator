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

protected:
	// Workers currently assigned to this building
	UPROPERTY()
	TArray<class ABaseVillager*> AssignedWorkers;
};
