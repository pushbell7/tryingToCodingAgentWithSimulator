// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseBuilding.h"
#include "House.generated.h"

/**
 * House building for villagers to rest and live
 * Can house multiple residents
 */
UCLASS()
class SIMULATOR_API AHouse : public ABaseBuilding
{
	GENERATED_BODY()

public:
	AHouse();

protected:
	virtual void BeginPlay() override;

public:
	// Maximum number of residents this house can support
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "House")
	int32 MaxResidents;

	// Current number of residents
	UPROPERTY(BlueprintReadOnly, Category = "House")
	int32 CurrentResidents;

	// Residents living in this house
	UPROPERTY()
	TArray<class ABaseVillager*> Residents;

	// Check if house has available space
	UFUNCTION(BlueprintCallable, Category = "House")
	bool HasAvailableSpace() const;

	// Add a resident to this house
	UFUNCTION(BlueprintCallable, Category = "House")
	bool AddResident(class ABaseVillager* Villager);

	// Remove a resident from this house
	UFUNCTION(BlueprintCallable, Category = "House")
	bool RemoveResident(class ABaseVillager* Villager);

	// Get number of available beds
	UFUNCTION(BlueprintCallable, Category = "House")
	int32 GetAvailableSpace() const;
};
