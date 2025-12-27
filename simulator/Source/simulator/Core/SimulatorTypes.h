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
