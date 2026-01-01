// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weaver.h"
#include "Components/StaticMeshComponent.h"

AWeaver::AWeaver()
{
	// Building identification
	BuildingType = EBuildingType::Weaver;
	BuildingName = TEXT("Weaver");

	// Production settings
	bCanProduce = true;
	bIsOperational = true;

	// Worker requirements
	MaxWorkers = 3;
	OptimalWorkerCount = 2;
	RequiredSkillLevel = ESkillLevel::Master; // Tier 3 requires master craftsmen

	// Production recipe: Currently no input defined (can be configured in blueprint)
	// In future: could use Leather or add new resource like "Wool" or "Flax"
	// For now: 10 Leather -> 5 Cloth (placeholder recipe)
	ProductionRecipe.InputResources.Add(FResourceStack(EResourceType::Leather, 10));
	ProductionRecipe.OutputResources.Add(FResourceStack(EResourceType::Cloth, 5));
	ProductionRecipe.CraftingTime = 60.0f; // Per turn
	ProductionRecipe.RequiredBuilding = EBuildingType::Weaver;

	// Construction cost (expensive Tier 3 building)
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Wood, 70));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Stone, 50));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Iron, 15));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Tools, 10));
	ConstructionCost.RequiredWorkAmount = 400.0f;
	ConstructionCost.MaxWorkers = 5;
}

void AWeaver::BeginPlay()
{
	Super::BeginPlay();
}
