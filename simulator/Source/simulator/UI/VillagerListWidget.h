// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SimulatorTypes.h"
#include "VillagerListWidget.generated.h"

/**
 * Villager information struct for UI display
 */
USTRUCT(BlueprintType)
struct FVillagerDisplayInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Villager")
	class ABaseVillager* VillagerRef;

	UPROPERTY(BlueprintReadOnly, Category = "Villager")
	FString VillagerName;

	UPROPERTY(BlueprintReadOnly, Category = "Villager")
	EActorState CurrentState;

	UPROPERTY(BlueprintReadOnly, Category = "Villager")
	ESocialClass SocialClass;

	UPROPERTY(BlueprintReadOnly, Category = "Villager")
	TMap<EBuildingType, ESkillLevel> Skills;

	UPROPERTY(BlueprintReadOnly, Category = "Villager")
	class ABaseBuilding* AssignedBuilding;

	UPROPERTY(BlueprintReadOnly, Category = "Villager")
	FString AssignedBuildingName;

	FVillagerDisplayInfo()
		: VillagerRef(nullptr)
		, VillagerName(TEXT(""))
		, CurrentState(EActorState::IDLE)
		, SocialClass(ESocialClass::Peasant)
		, AssignedBuilding(nullptr)
		, AssignedBuildingName(TEXT("Unemployed"))
	{}
};

/**
 * Widget for displaying villager list and managing assignments
 * Base C++ class - inherit in Blueprint to design visual layout
 *
 * Usage in Blueprint (WBP_VillagerList):
 * 1. Create Widget Blueprint inheriting from this class
 * 2. Add ListView or ScrollBox to display villagers
 * 3. Call SetTerritory() to bind to a territory
 * 4. Call UpdateVillagerList() to refresh data
 * 5. Implement OnVillagerListUpdated event to populate list
 * 6. Use GetVillagerSkillText() to display skills
 */
UCLASS()
class SIMULATOR_API UVillagerListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Set territory to display villagers from
	UFUNCTION(BlueprintCallable, Category = "Villager List")
	void SetTerritory(class ATerritory* InTerritory);

	// Update villager list
	UFUNCTION(BlueprintCallable, Category = "Villager List")
	void UpdateVillagerList();

	// Get all villagers as array
	UFUNCTION(BlueprintCallable, Category = "Villager List")
	TArray<FVillagerDisplayInfo> GetVillagerList() const;

	// Get skill level text for a villager and building type
	UFUNCTION(BlueprintCallable, Category = "Villager List")
	FString GetVillagerSkillText(class ABaseVillager* Villager, EBuildingType BuildingType) const;

	// Get highest skill level for a villager
	UFUNCTION(BlueprintCallable, Category = "Villager List")
	ESkillLevel GetHighestSkillLevel(class ABaseVillager* Villager) const;

	// Check if villager can work at building
	UFUNCTION(BlueprintCallable, Category = "Villager List")
	bool CanVillagerWorkAt(class ABaseVillager* Villager, class ABaseBuilding* Building) const;

	// Assign villager to building
	UFUNCTION(BlueprintCallable, Category = "Villager List")
	bool AssignVillagerToBuilding(class ABaseVillager* Villager, class ABaseBuilding* Building);

	// Remove villager from current building
	UFUNCTION(BlueprintCallable, Category = "Villager List")
	bool UnassignVillager(class ABaseVillager* Villager);

protected:
	// Called when villager list is updated
	UFUNCTION(BlueprintImplementableEvent, Category = "Villager List")
	void OnVillagerListUpdated();

	// Target territory
	UPROPERTY(BlueprintReadOnly, Category = "Villager List")
	class ATerritory* TargetTerritory;

	// Cached villager list
	UPROPERTY(BlueprintReadOnly, Category = "Villager List")
	TArray<FVillagerDisplayInfo> CachedVillagers;

	// Total villager count
	UPROPERTY(BlueprintReadOnly, Category = "Villager List")
	int32 TotalVillagerCount;

	// Employed villager count
	UPROPERTY(BlueprintReadOnly, Category = "Villager List")
	int32 EmployedVillagerCount;
};
