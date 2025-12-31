// Copyright Epic Games, Inc. All Rights Reserved.

#include "GuildHall.h"
#include "BaseVillager.h"
#include "Territory.h"

AGuildHall::AGuildHall()
{
	BuildingType = EBuildingType::GuildHall;
	BuildingName = TEXT("Guild Hall");

	MaxWorkers = 1; // Guild master only
	bCanProduce = false;
	RequiredSkillLevel = ESkillLevel::Master; // Need master to run guild

	bIsTraining = false;
	CostMultiplier = 1.0f;
	BaseDurationTurns = 5; // 5 turns (5 minutes) base training
}

void AGuildHall::BeginPlay()
{
	Super::BeginPlay();
}

bool AGuildHall::StartTraining(ABaseVillager* Villager, EBuildingType Profession, ESkillLevel TargetLevel)
{
	if (!Villager)
	{
		UE_LOG(LogTemp, Warning, TEXT("GuildHall: Cannot train null villager"));
		return false;
	}

	if (!OwnerTerritory)
	{
		UE_LOG(LogTemp, Warning, TEXT("GuildHall: No owner territory"));
		return false;
	}

	// Check if guild has a master (worker assigned)
	if (CurrentWorkers < 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("GuildHall: No guild master assigned"));
		return false;
	}

	ESkillLevel CurrentLevel = Villager->GetSkillLevel(Profession);

	// Can't train to lower or same level
	if (TargetLevel <= CurrentLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("GuildHall: Villager %s already has skill level %d for %s"),
			*Villager->VillagerName,
			(int32)CurrentLevel,
			*UEnum::GetValueAsString(Profession));
		return false;
	}

	// Can only train one level at a time
	if (static_cast<int32>(TargetLevel) > static_cast<int32>(CurrentLevel) + 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("GuildHall: Cannot skip skill levels (must train %d -> %d first)"),
			(int32)CurrentLevel,
			(int32)CurrentLevel + 1);
		return false;
	}

	// Check training cost
	if (!CanAffordTraining(CurrentLevel, TargetLevel))
	{
		UE_LOG(LogTemp, Warning, TEXT("GuildHall: Insufficient resources for training"));
		return false;
	}

	// Pay training cost
	TMap<EResourceType, int32> Cost = GetTrainingCost(CurrentLevel, TargetLevel);
	for (const auto& Pair : Cost)
	{
		OwnerTerritory->RemoveResource(Pair.Key, Pair.Value);
	}

	// Create training request
	FTrainingRequest Request;
	Request.Trainee = Villager;
	Request.Profession = Profession;
	Request.TargetLevel = TargetLevel;
	Request.Progress = 0.0f;
	Request.TotalTurns = GetTrainingDuration(CurrentLevel, TargetLevel);
	Request.TurnsCompleted = 0;

	// If not currently training, start immediately
	if (!bIsTraining)
	{
		CurrentTraining = Request;
		bIsTraining = true;

		UE_LOG(LogTemp, Log, TEXT("GuildHall: Started training %s for %s (%d -> %d) - %d turns"),
			*Villager->VillagerName,
			*UEnum::GetValueAsString(Profession),
			(int32)CurrentLevel,
			(int32)TargetLevel,
			Request.TotalTurns);
	}
	else
	{
		// Add to queue
		TrainingQueue.Add(Request);

		UE_LOG(LogTemp, Log, TEXT("GuildHall: Queued training for %s (%d in queue)"),
			*Villager->VillagerName,
			TrainingQueue.Num());
	}

	return true;
}

void AGuildHall::ProcessTrainingTurn()
{
	if (!bIsTraining)
		return;

	if (!CurrentTraining.Trainee)
	{
		UE_LOG(LogTemp, Warning, TEXT("GuildHall: Training cancelled - trainee is null"));
		bIsTraining = false;
		return;
	}

	// Increment progress
	CurrentTraining.TurnsCompleted++;
	CurrentTraining.Progress = (float)CurrentTraining.TurnsCompleted / (float)CurrentTraining.TotalTurns;

	UE_LOG(LogTemp, Log, TEXT("GuildHall: Training progress %s - %d/%d turns (%.0f%%)"),
		*CurrentTraining.Trainee->VillagerName,
		CurrentTraining.TurnsCompleted,
		CurrentTraining.TotalTurns,
		CurrentTraining.Progress * 100.0f);

	// Check if training complete
	if (CurrentTraining.TurnsCompleted >= CurrentTraining.TotalTurns)
	{
		CompleteTraining();
	}
}

void AGuildHall::CompleteTraining()
{
	if (!CurrentTraining.Trainee)
		return;

	// Upgrade skill
	CurrentTraining.Trainee->SetSkillLevel(CurrentTraining.Profession, CurrentTraining.TargetLevel);

	UE_LOG(LogTemp, Warning, TEXT("GuildHall: TRAINING COMPLETE - %s mastered %s at level %d!"),
		*CurrentTraining.Trainee->VillagerName,
		*UEnum::GetValueAsString(CurrentTraining.Profession),
		(int32)CurrentTraining.TargetLevel);

	// Clear current training
	bIsTraining = false;
	CurrentTraining = FTrainingRequest();

	// Start next queued training
	if (TrainingQueue.Num() > 0)
	{
		CurrentTraining = TrainingQueue[0];
		TrainingQueue.RemoveAt(0);
		bIsTraining = true;

		UE_LOG(LogTemp, Log, TEXT("GuildHall: Starting next training from queue - %s"),
			*CurrentTraining.Trainee->VillagerName);
	}
}

void AGuildHall::CancelTraining()
{
	if (!bIsTraining)
		return;

	UE_LOG(LogTemp, Warning, TEXT("GuildHall: Training cancelled - %s"),
		CurrentTraining.Trainee ? *CurrentTraining.Trainee->VillagerName : TEXT("Unknown"));

	bIsTraining = false;
	CurrentTraining = FTrainingRequest();

	// Start next queued training
	if (TrainingQueue.Num() > 0)
	{
		CurrentTraining = TrainingQueue[0];
		TrainingQueue.RemoveAt(0);
		bIsTraining = true;
	}
}

TMap<EResourceType, int32> AGuildHall::GetTrainingCost(ESkillLevel CurrentLevel, ESkillLevel TargetLevel) const
{
	TMap<EResourceType, int32> Cost;

	// Base cost increases with target level
	int32 BaseCost = 0;

	switch (TargetLevel)
	{
	case ESkillLevel::Novice:
		BaseCost = 0; // Free (everyone starts as novice)
		break;
	case ESkillLevel::Apprentice:
		BaseCost = 50; // Novice -> Apprentice: 50 food
		Cost.Add(EResourceType::Food, BaseCost);
		break;
	case ESkillLevel::Journeyman:
		BaseCost = 100; // Apprentice -> Journeyman: 100 food + 50 tools
		Cost.Add(EResourceType::Food, BaseCost);
		Cost.Add(EResourceType::Tools, 50);
		break;
	case ESkillLevel::Master:
		BaseCost = 200; // Journeyman -> Master: 200 food + 100 tools + 50 gold
		Cost.Add(EResourceType::Food, BaseCost);
		Cost.Add(EResourceType::Tools, 100);
		Cost.Add(EResourceType::Gold, 50);
		break;
	}

	return Cost;
}

int32 AGuildHall::GetTrainingDuration(ESkillLevel CurrentLevel, ESkillLevel TargetLevel) const
{
	// Duration increases with target level
	switch (TargetLevel)
	{
	case ESkillLevel::Novice:
		return 0; // Instant (default)
	case ESkillLevel::Apprentice:
		return BaseDurationTurns * 1; // 5 turns (5 minutes)
	case ESkillLevel::Journeyman:
		return BaseDurationTurns * 2; // 10 turns (10 minutes)
	case ESkillLevel::Master:
		return BaseDurationTurns * 4; // 20 turns (20 minutes)
	default:
		return BaseDurationTurns;
	}
}

bool AGuildHall::CanAffordTraining(ESkillLevel CurrentLevel, ESkillLevel TargetLevel) const
{
	if (!OwnerTerritory)
		return false;

	TMap<EResourceType, int32> Cost = GetTrainingCost(CurrentLevel, TargetLevel);

	for (const auto& Pair : Cost)
	{
		if (!OwnerTerritory->HasResource(Pair.Key, Pair.Value))
		{
			return false;
		}
	}

	return true;
}
