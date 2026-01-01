// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mill.h"
#include "Components/StaticMeshComponent.h"

AMill::AMill()
{
	// Building identification
	BuildingType = EBuildingType::Mill;
	BuildingName = TEXT("Mill");

	// Production settings
	bCanProduce = true;
	bIsOperational = true;

	// Worker requirements
	MaxWorkers = 3;
	OptimalWorkerCount = 2;
	RequiredSkillLevel = ESkillLevel::Apprentice; // Tier 2 requires training

	// Production recipe: 10 Food -> 8 Bread
	ProductionRecipe.InputResources.Add(FResourceStack(EResourceType::Food, 10));
	ProductionRecipe.OutputResources.Add(FResourceStack(EResourceType::Bread, 8));
	ProductionRecipe.CraftingTime = 60.0f; // Per turn
	ProductionRecipe.RequiredBuilding = EBuildingType::Mill;

	// Construction cost
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Wood, 50));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Stone, 30));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Iron, 10));
	ConstructionCost.RequiredWorkAmount = 200.0f;
	ConstructionCost.MaxWorkers = 5;
}

void AMill::BeginPlay()
{
	Super::BeginPlay();
}
