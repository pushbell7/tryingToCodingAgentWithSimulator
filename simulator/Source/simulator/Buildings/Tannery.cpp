// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tannery.h"
#include "Components/StaticMeshComponent.h"

ATannery::ATannery()
{
	// Building identification
	BuildingType = EBuildingType::Tannery;
	BuildingName = TEXT("Tannery");

	// Production settings
	bCanProduce = true;
	bIsOperational = true;

	// Worker requirements
	MaxWorkers = 3;
	OptimalWorkerCount = 2;
	RequiredSkillLevel = ESkillLevel::Apprentice; // Tier 2 requires training

	// Production recipe: 8 Meat -> 5 Leather
	ProductionRecipe.InputResources.Add(FResourceStack(EResourceType::Meat, 8));
	ProductionRecipe.OutputResources.Add(FResourceStack(EResourceType::Leather, 5));
	ProductionRecipe.CraftingTime = 60.0f; // Per turn
	ProductionRecipe.RequiredBuilding = EBuildingType::Tannery;

	// Construction cost
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Wood, 45));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Stone, 35));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Iron, 10));
	ConstructionCost.RequiredWorkAmount = 200.0f;
	ConstructionCost.MaxWorkers = 5;
}

void ATannery::BeginPlay()
{
	Super::BeginPlay();
}
