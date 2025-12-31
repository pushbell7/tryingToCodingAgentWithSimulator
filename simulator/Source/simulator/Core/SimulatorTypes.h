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
 * Skill level for production buildings (medieval guild system)
 * Novice: Can work in Tier 1 buildings (raw material gathering)
 * Apprentice: Can work in Tier 2 buildings (basic processing)
 * Journeyman: Can work in Tier 2 buildings with better efficiency
 * Master: Can work in Tier 3 buildings (luxury goods) and train others
 */
UENUM(BlueprintType)
enum class ESkillLevel : uint8
{
	Novice      UMETA(DisplayName = "Novice"),         // Unskilled labor (Tier 1)
	Apprentice  UMETA(DisplayName = "Apprentice"),     // Learning trade (Tier 2 entry)
	Journeyman  UMETA(DisplayName = "Journeyman"),     // Skilled worker (Tier 2)
	Master      UMETA(DisplayName = "Master")          // Expert craftsman (Tier 3 + training)
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
 * Construction cost for buildings
 * Defines resources and labor required to construct a building
 */
USTRUCT(BlueprintType)
struct FConstructionCost
{
	GENERATED_BODY()

	// Resources required for construction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	TArray<FResourceStack> RequiredResources;

	// Amount of work required (in work units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	float RequiredWorkAmount;

	// Maximum number of workers that can work simultaneously
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	int32 MaxWorkers;

	FConstructionCost()
		: RequiredWorkAmount(100.0f)
		, MaxWorkers(5)
	{}

	FConstructionCost(TArray<FResourceStack> InResources, float InWork, int32 InWorkers)
		: RequiredResources(InResources)
		, RequiredWorkAmount(InWork)
		, MaxWorkers(InWorkers)
	{}
};

/**
 * Noble rank based on territory count
 * Knight (1+), Baron (3+), Count (7+), Duke (15+), King (30+)
 */
UENUM(BlueprintType)
enum class ENobleRank : uint8
{
	None        UMETA(DisplayName = "None"),           // No territories
	Knight      UMETA(DisplayName = "Knight"),         // 1+ territories
	Baron       UMETA(DisplayName = "Baron"),          // 3+ territories
	Count       UMETA(DisplayName = "Count"),          // 7+ territories
	Duke        UMETA(DisplayName = "Duke"),           // 15+ territories
	King        UMETA(DisplayName = "King")            // 30+ territories
};

/**
 * Territory state
 */
UENUM(BlueprintType)
enum class ETerritoryState : uint8
{
	Neutral         UMETA(DisplayName = "Neutral"),        // No owner, resources/population decaying
	Owned           UMETA(DisplayName = "Owned"),          // Under faction control
	UnderSiege      UMETA(DisplayName = "Under Siege")     // Landmark being attacked
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

	// Tier 1 Production (Raw materials - labor only)
	Farm            UMETA(DisplayName = "Farm"),           // Produces Food
	Pasture         UMETA(DisplayName = "Pasture"),        // Produces Meat
	Lumbercamp      UMETA(DisplayName = "Lumbercamp"),     // Produces Wood
	Quarry          UMETA(DisplayName = "Quarry"),         // Produces Stone
	IronMine        UMETA(DisplayName = "Iron Mine"),      // Produces Iron
	FishingHut      UMETA(DisplayName = "Fishing Hut"),    // Produces Fish

	// Tier 2 Processing (Raw -> Processed)
	Mill            UMETA(DisplayName = "Mill"),           // Food -> Bread
	Bakery          UMETA(DisplayName = "Bakery"),         // Food -> Bread (alternative)
	Tannery         UMETA(DisplayName = "Tannery"),        // Meat -> Leather
	Sawmill         UMETA(DisplayName = "Sawmill"),        // Wood -> Planks
	Blacksmith      UMETA(DisplayName = "Blacksmith"),     // Iron + Wood -> Tools/Weapons

	// Tier 3 Processing (Processed -> Luxury)
	Brewery         UMETA(DisplayName = "Brewery"),        // Food -> Ale
	Weaver          UMETA(DisplayName = "Weaver"),         // -> Cloth

	// Special
	Market          UMETA(DisplayName = "Market"),         // Trade hub
	House           UMETA(DisplayName = "House"),          // Residential (population capacity)
	TownHall        UMETA(DisplayName = "Town Hall"),      // Administration building
	GuildHall       UMETA(DisplayName = "Guild Hall"),     // Skill training (medieval apprenticeship)
	Landmark        UMETA(DisplayName = "Landmark")        // Territory ownership marker
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
