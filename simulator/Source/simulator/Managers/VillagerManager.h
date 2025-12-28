// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimulatorTypes.h"
#include "VillagerManager.generated.h"

/**
 * Manager for all villagers in the world
 * Handles automatic assignment of homes and work zones
 * Tracks population statistics and manages villager lifecycle
 */
UCLASS()
class SIMULATOR_API AVillagerManager : public AActor
{
	GENERATED_BODY()

public:
	AVillagerManager();

protected:
	virtual void BeginPlay() override;

public:
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Villager Manager")
	float RefreshInterval;

	// Should auto-assign on BeginPlay?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Villager Manager")
	bool bAutoAssignOnStart;

	// Timer for periodic refresh
	FTimerHandle RefreshTimerHandle;

	// Periodic refresh function
	void PeriodicRefresh();
};
