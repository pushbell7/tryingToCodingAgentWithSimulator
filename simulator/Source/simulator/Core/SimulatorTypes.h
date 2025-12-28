// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SimulatorTypes.generated.h"

/**
 * Actor state in the simulation
 */
UENUM(BlueprintType)
enum class EActorState : uint8
{
	IDLE        UMETA(DisplayName = "Idle"),           // Waiting for action permission
	MOVING      UMETA(DisplayName = "Moving"),         // Traveling to destination
	WORKING     UMETA(DisplayName = "Working"),        // Performing job (farming, smithing, etc)
	FIGHTING    UMETA(DisplayName = "Fighting"),       // In combat
	TRADING     UMETA(DisplayName = "Trading"),        // Trading with merchant
	RESTING     UMETA(DisplayName = "Resting"),        // Sleeping/recovering
	DEAD        UMETA(DisplayName = "Dead")            // Dead, awaiting removal
};

/**
 * Social class/rank for priority system
 */
UENUM(BlueprintType)
enum class ESocialClass : uint8
{
	Peasant     UMETA(DisplayName = "Peasant"),        // Lowest priority
	Commoner    UMETA(DisplayName = "Commoner"),       // Normal citizens
	Merchant    UMETA(DisplayName = "Merchant"),       // Traders
	Soldier     UMETA(DisplayName = "Soldier"),        // Guards/military
	Noble       UMETA(DisplayName = "Noble"),          // Nobility
	Lord        UMETA(DisplayName = "Lord")            // Highest priority
};

/**
 * Action types that require turn permission
 */
UENUM(BlueprintType)
enum class EActionType : uint8
{
	None        UMETA(DisplayName = "None"),
	Move        UMETA(DisplayName = "Move"),           // Long distance travel
	Work        UMETA(DisplayName = "Work"),           // Perform job
	Fight       UMETA(DisplayName = "Fight"),          // Engage in combat
	Trade       UMETA(DisplayName = "Trade"),          // Trade goods
	Rest        UMETA(DisplayName = "Rest")            // Sleep/recover
};

/**
 * Terrain zone types for resource management
 */
UENUM(BlueprintType)
enum class ETerrainZone : uint8
{
	Farmland    UMETA(DisplayName = "Farmland"),       // Agricultural land (crops, orchards)
	Pasture     UMETA(DisplayName = "Pasture"),        // Livestock grazing
	Forest      UMETA(DisplayName = "Forest"),         // Timber, hunting
	Mountain    UMETA(DisplayName = "Mountain"),       // Mining (ore, stone)
	Water       UMETA(DisplayName = "Water"),          // Fishing
	Settlement  UMETA(DisplayName = "Settlement")      // Buildings, markets
};

/**
 * Resource types in the economy system
 * Tier 1: Raw materials from zones
 * Tier 2: Processed goods from workshops
 * Tier 3: Luxury/trade goods
 */
UENUM(BlueprintType)
enum class EResourceType : uint8
{
	// Tier 1 - Raw Materials
	Food        UMETA(DisplayName = "Food"),           // From Farmland (wheat, vegetables)
	Meat        UMETA(DisplayName = "Meat"),           // From Pasture (livestock)
	Wood        UMETA(DisplayName = "Wood"),           // From Forest (timber)
	Stone       UMETA(DisplayName = "Stone"),          // From Mountain (quarry)
	Iron        UMETA(DisplayName = "Iron"),           // From Mountain (mine)
	Fish        UMETA(DisplayName = "Fish"),           // From Water (fishing)

	// Tier 2 - Processed Goods
	Bread       UMETA(DisplayName = "Bread"),          // Food -> Bread (mill/bakery)
	Leather     UMETA(DisplayName = "Leather"),        // Meat -> Leather (tannery)
	Planks      UMETA(DisplayName = "Planks"),         // Wood -> Planks (sawmill)
	Tools       UMETA(DisplayName = "Tools"),          // Iron + Wood -> Tools (blacksmith)
	Weapons     UMETA(DisplayName = "Weapons"),        // Iron -> Weapons (blacksmith)

	// Tier 3 - Luxury/Trade Goods
	Gold        UMETA(DisplayName = "Gold"),           // From Mountain (rare)
	Cloth       UMETA(DisplayName = "Cloth"),          // Processed (loom)
	Ale         UMETA(DisplayName = "Ale")             // Food -> Ale (brewery)
};

/**
 * Single resource stack (type + quantity)
 */
USTRUCT(BlueprintType)
struct FResourceStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EResourceType ResourceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 Quantity;

	FResourceStack()
		: ResourceType(EResourceType::Food)
		, Quantity(0)
	{}

	FResourceStack(EResourceType InType, int32 InQuantity)
		: ResourceType(InType)
		, Quantity(InQuantity)
	{}
};

/**
 * Building types for settlement zones
 */
UENUM(BlueprintType)
enum class EBuildingType : uint8
{
	// Storage
	Warehouse       UMETA(DisplayName = "Warehouse"),      // General storage for all resources
	Granary         UMETA(DisplayName = "Granary"),        // Food storage only

	// Processing - Tier 1 to Tier 2
	Mill            UMETA(DisplayName = "Mill"),           // Food -> Bread
	Bakery          UMETA(DisplayName = "Bakery"),         // Food -> Bread (alternative)
	Tannery         UMETA(DisplayName = "Tannery"),        // Meat -> Leather
	Sawmill         UMETA(DisplayName = "Sawmill"),        // Wood -> Planks
	Blacksmith      UMETA(DisplayName = "Blacksmith"),     // Iron + Wood -> Tools/Weapons

	// Processing - Tier 2 to Tier 3
	Brewery         UMETA(DisplayName = "Brewery"),        // Food -> Ale
	Weaver          UMETA(DisplayName = "Weaver"),         // -> Cloth

	// Special
	Market          UMETA(DisplayName = "Market"),         // Trade hub
	House           UMETA(DisplayName = "House"),          // Residential (population capacity)
	TownHall        UMETA(DisplayName = "Town Hall")       // Administration building
};

/**
 * Crafting recipe - defines input and output for resource processing
 */
USTRUCT(BlueprintType)
struct FCraftingRecipe
{
	GENERATED_BODY()

	// Input resources required
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TArray<FResourceStack> InputResources;

	// Output resources produced
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TArray<FResourceStack> OutputResources;

	// Time to craft in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	float CraftingTime;

	// Building type required for this recipe
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	EBuildingType RequiredBuilding;

	FCraftingRecipe()
		: CraftingTime(5.0f)
		, RequiredBuilding(EBuildingType::Blacksmith)
	{}
};
