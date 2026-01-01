// Copyright Epic Games, Inc. All Rights Reserved.

#include "Blacksmith.h"
#include "Components/StaticMeshComponent.h"

ABlacksmith::ABlacksmith()
{
	// Building identification
	BuildingType = EBuildingType::Blacksmith;
	BuildingName = TEXT("Blacksmith");

	// Production settings
	bCanProduce = true;
	bIsOperational = true;

	// Worker requirements
	MaxWorkers = 4;
	OptimalWorkerCount = 2;
	RequiredSkillLevel = ESkillLevel::Journeyman; // Skilled craftsmen required

	// Production recipe: 5 Iron + 5 Wood -> 4 Tools
	// Note: Can be changed to produce Weapons in blueprint/instance
	ProductionRecipe.InputResources.Add(FResourceStack(EResourceType::Iron, 5));
	ProductionRecipe.InputResources.Add(FResourceStack(EResourceType::Wood, 5));
	ProductionRecipe.OutputResources.Add(FResourceStack(EResourceType::Tools, 4));
	ProductionRecipe.CraftingTime = 60.0f; // Per turn
	ProductionRecipe.RequiredBuilding = EBuildingType::Blacksmith;

	// Construction cost
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Wood, 40));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Stone, 60));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Iron, 30));
	ConstructionCost.RequiredWorkAmount = 300.0f;
	ConstructionCost.MaxWorkers = 5;
}

void ABlacksmith::BeginPlay()
{
	Super::BeginPlay();
}
