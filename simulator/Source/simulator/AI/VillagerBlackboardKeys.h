// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Blackboard key names used by villager AI
 */
namespace VillagerBlackboardKeys
{
	// Target locations
	const FName TargetLocation = TEXT("TargetLocation");
	const FName HomeLocation = TEXT("HomeLocation");
	const FName WorkLocation = TEXT("WorkLocation");
	const FName PatrolLocation = TEXT("PatrolLocation");

	// Target actors
	const FName TargetActor = TEXT("TargetActor");
	const FName EnemyTarget = TEXT("EnemyTarget");

	// States
	const FName IsWorking = TEXT("IsWorking");
	const FName IsPatrolling = TEXT("IsPatrolling");
	const FName IsResting = TEXT("IsResting");
	const FName HasTarget = TEXT("HasTarget");

	// Self reference
	const FName SelfActor = TEXT("SelfActor");
}
