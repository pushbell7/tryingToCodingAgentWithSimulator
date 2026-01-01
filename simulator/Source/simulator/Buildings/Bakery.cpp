// Copyright Epic Games, Inc. All Rights Reserved.

#include "Bakery.h"
#include "Components/StaticMeshComponent.h"

ABakery::ABakery()
{
	// Building identification
	BuildingType = EBuildingType::Bakery;
	BuildingName = TEXT("Bakery");

	// Production settings
	bCanProduce = true;
	bIsOperational = true;

	// Worker requirements
	MaxWorkers = 4;
	OptimalWorkerCount = 3;
	RequiredSkillLevel = ESkillLevel::Apprentice; // Tier 2 requires training

	// Production recipe: 10 Food -> 10 Bread (better than Mill)
	ProductionRecipe.InputResources.Add(FResourceStack(EResourceType::Food, 10));
	ProductionRecipe.OutputResources.Add(FResourceStack(EResourceType::Bread, 10));
	ProductionRecipe.CraftingTime = 60.0f; // Per turn
	ProductionRecipe.RequiredBuilding = EBuildingType::Bakery;

	// Construction cost (more expensive than Mill)
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Wood, 60));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Stone, 40));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Iron, 15));
	ConstructionCost.RequiredWorkAmount = 250.0f;
	ConstructionCost.MaxWorkers = 5;
}

void ABakery::BeginPlay()
{
	Super::BeginPlay();
}
