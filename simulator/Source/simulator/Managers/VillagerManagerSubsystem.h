// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SimulatorTypes.h"
#include "VillagerManagerSubsystem.generated.h"

/**
 * Manager for all villagers in the world as a WorldSubsystem
 * Handles automatic assignment of homes and work zones
 * Tracks population statistics and manages villager lifecycle
 */
UCLASS()
class SIMULATOR_API UVillagerManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Refresh the list of all villagers in the world
	UFUNCTION(BlueprintCallable, Category = "Villager Manager")
	void RefreshVillagerList();

	// Auto-assign homes and work zones to all villagers
	UFUNCTION(BlueprintCallable, Category = "Villager Manager")
	void AutoAssignAll();

	// Auto-assign home to a specific villager
	UFUNCTION(BlueprintCallable, Category = "Villager Manager")
	bool AutoAssignHome(class ABaseVillager* Villager);

	// Auto-assign work zone to a specific villager
	UFUNCTION(BlueprintCallable, Category = "Villager Manager")
	bool AutoAssignWorkZone(class ABaseVillager* Villager);

	// Get all villagers
	UFUNCTION(BlueprintCallable, Category = "Villager Manager")
	TArray<class ABaseVillager*> GetAllVillagers() const { return AllVillagers; }

	// Get villagers by role
	UFUNCTION(BlueprintCallable, Category = "Villager Manager")
	TArray<class ABaseVillager*> GetVillagersByRole(EVillagerRole VillagerRole) const;

	// Get villagers by social class
	UFUNCTION(BlueprintCallable, Category = "Villager Manager")
	TArray<class ABaseVillager*> GetVillagersBySocialClass(ESocialClass VillagerSocialClass) const;

	// Get homeless villagers
	UFUNCTION(BlueprintCallable, Category = "Villager Manager")
	TArray<class ABaseVillager*> GetHomelessVillagers() const;

	// Get unemployed villagers (no work zone assigned)
	UFUNCTION(BlueprintCallable, Category = "Villager Manager")
	TArray<class ABaseVillager*> GetUnemployedVillagers() const;

	// Get population statistics
	UFUNCTION(BlueprintCallable, Category = "Villager Manager")
	int32 GetTotalPopulation() const { return AllVillagers.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Villager Manager")
	int32 GetPopulationByRole(EVillagerRole VillagerRole) const;

	UFUNCTION(BlueprintCallable, Category = "Villager Manager")
	int32 GetHomelessCount() const;

	UFUNCTION(BlueprintCallable, Category = "Villager Manager")
	int32 GetUnemployedCount() const;

protected:
	// Cache of all villagers in the world
	UPROPERTY()
	TArray<class ABaseVillager*> AllVillagers;

	// How often to refresh the villager list (in seconds)
	float RefreshInterval;

	// Should auto-assign on initialization?
	bool bAutoAssignOnStart;

	// Timer for periodic refresh
	FTimerHandle RefreshTimerHandle;

	// Periodic refresh function
	void PeriodicRefresh();
};
