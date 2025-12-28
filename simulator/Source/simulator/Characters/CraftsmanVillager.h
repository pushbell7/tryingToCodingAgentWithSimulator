// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseVillager.h"
#include "SimulatorTypes.h"
#include "CraftsmanVillager.generated.h"

/**
 * Craftsman villager specialized in processing resources
 * Works at workshops to convert raw materials into processed goods
 */
UCLASS()
class SIMULATOR_API ACraftsmanVillager : public ABaseVillager
{
	GENERATED_BODY()

public:
	ACraftsmanVillager();

protected:
	virtual void BeginPlay() override;

public:
	// Craftsman specialty (which building type they work at)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craftsman")
	EBuildingType Specialty;

	// Assigned workshop
	UPROPERTY(BlueprintReadWrite, Category = "Craftsman")
	class ABaseBuilding* AssignedWorkshop;

	// Crafting efficiency (1.0 = normal, higher = faster)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Craftsman")
	float CraftingEfficiency;

	// Assign this craftsman to a workshop
	UFUNCTION(BlueprintCallable, Category = "Craftsman")
	bool AssignToWorkshop(class ABaseBuilding* Workshop);

	// Unassign from current workshop
	UFUNCTION(BlueprintCallable, Category = "Craftsman")
	void UnassignFromWorkshop();

	// Check if craftsman is assigned to a workshop
	UFUNCTION(BlueprintCallable, Category = "Craftsman")
	bool IsAssigned() const;
};
