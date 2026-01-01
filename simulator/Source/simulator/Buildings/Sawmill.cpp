// Copyright Epic Games, Inc. All Rights Reserved.

#include "Sawmill.h"
#include "Components/StaticMeshComponent.h"

ASawmill::ASawmill()
{
	// Building identification
	BuildingType = EBuildingType::Sawmill;
	BuildingName = TEXT("Sawmill");

	// Production settings
	bCanProduce = true;
	bIsOperational = true;

	// Worker requirements
	MaxWorkers = 4;
	OptimalWorkerCount = 2;
	RequiredSkillLevel = ESkillLevel::Apprentice; // Tier 2 requires training

	// Production recipe: 10 Wood -> 8 Planks
	ProductionRecipe.InputResources.Add(FResourceStack(EResourceType::Wood, 10));
	ProductionRecipe.OutputResources.Add(FResourceStack(EResourceType::Planks, 8));
	ProductionRecipe.CraftingTime = 60.0f; // Per turn
	ProductionRecipe.RequiredBuilding = EBuildingType::Sawmill;

	// Construction cost
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Wood, 50));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Stone, 30));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Iron, 15));
	ConstructionCost.RequiredWorkAmount = 200.0f;
	ConstructionCost.MaxWorkers = 5;
}

void ASawmill::BeginPlay()
{
	Super::BeginPlay();
}
