// Copyright Epic Games, Inc. All Rights Reserved.

#include "Brewery.h"
#include "Components/StaticMeshComponent.h"

ABrewery::ABrewery()
{
	// Building identification
	BuildingType = EBuildingType::Brewery;
	BuildingName = TEXT("Brewery");

	// Production settings
	bCanProduce = true;
	bIsOperational = true;

	// Worker requirements
	MaxWorkers = 3;
	OptimalWorkerCount = 2;
	RequiredSkillLevel = ESkillLevel::Master; // Tier 3 requires master craftsmen

	// Production recipe: 15 Food -> 5 Ale (luxury good - low output, high value)
	ProductionRecipe.InputResources.Add(FResourceStack(EResourceType::Food, 15));
	ProductionRecipe.OutputResources.Add(FResourceStack(EResourceType::Ale, 5));
	ProductionRecipe.CraftingTime = 60.0f; // Per turn
	ProductionRecipe.RequiredBuilding = EBuildingType::Brewery;

	// Construction cost (expensive Tier 3 building)
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Wood, 80));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Stone, 60));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Iron, 20));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Tools, 10));
	ConstructionCost.RequiredWorkAmount = 400.0f;
	ConstructionCost.MaxWorkers = 5;
}

void ABrewery::BeginPlay()
{
	Super::BeginPlay();
}
