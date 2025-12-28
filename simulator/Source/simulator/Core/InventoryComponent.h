// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SimulatorTypes.h"
#include "InventoryComponent.generated.h"

/**
 * Component for managing resource inventory
 * Can be attached to Villagers, Buildings, or Storage
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SIMULATOR_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

public:
	// Add resources to inventory (returns amount actually added)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddResource(EResourceType ResourceType, int32 Quantity);

	// Remove resources from inventory (returns amount actually removed)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 RemoveResource(EResourceType ResourceType, int32 Quantity);

	// Check if inventory has enough of a resource
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasResource(EResourceType ResourceType, int32 Quantity) const;

	// Get quantity of a specific resource
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetResourceQuantity(EResourceType ResourceType) const;

	// Get all resources in inventory
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FResourceStack> GetAllResources() const;

	// Clear all resources
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearInventory();

	// Get current total items in inventory
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetTotalItems() const;

	// Check if inventory is full
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool IsFull() const;

	// Get remaining capacity
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetRemainingCapacity() const;

public:
	// Maximum capacity (0 = unlimited)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 MaxCapacity;

protected:
	// Resource storage (ResourceType -> Quantity)
	UPROPERTY()
	TMap<EResourceType, int32> Resources;
};
