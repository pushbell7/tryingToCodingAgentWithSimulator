// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrainingQueueWidget.h"
#include "GuildHall.h"
#include "BaseVillager.h"

void UTrainingQueueWidget::SetGuildHall(AGuildHall* InGuildHall)
{
	TargetGuildHall = InGuildHall;
	UpdateTrainingQueue();
}

void UTrainingQueueWidget::UpdateTrainingQueue()
{
	if (!TargetGuildHall)
	{
		UE_LOG(LogTemp, Warning, TEXT("TrainingQueueWidget: No guild hall set"));
		return;
	}

	CachedQueue.Empty();
	bIsTraining = TargetGuildHall->bIsTraining;

	// Add current training if active
	if (bIsTraining && TargetGuildHall->CurrentTraining.Trainee)
	{
		FTrainingDisplayInfo Info;
		Info.VillagerName = TargetGuildHall->CurrentTraining.Trainee->VillagerName;
		Info.Profession = TargetGuildHall->CurrentTraining.Profession;
		Info.ProfessionName = UEnum::GetDisplayValueAsText(TargetGuildHall->CurrentTraining.Profession).ToString();
		Info.CurrentLevel = TargetGuildHall->CurrentTraining.Trainee->GetSkillLevel(Info.Profession);
		Info.TargetLevel = TargetGuildHall->CurrentTraining.TargetLevel;
		Info.Progress = TargetGuildHall->CurrentTraining.Progress;
		Info.TurnsCompleted = TargetGuildHall->CurrentTraining.TurnsCompleted;
		Info.TotalTurns = TargetGuildHall->CurrentTraining.TotalTurns;
		Info.bIsActive = true;

		CachedQueue.Add(Info);
	}

	// Add queued trainings
	for (const auto& Request : TargetGuildHall->TrainingQueue)
	{
		if (!Request.Trainee)
			continue;

		FTrainingDisplayInfo Info;
		Info.VillagerName = Request.Trainee->VillagerName;
		Info.Profession = Request.Profession;
		Info.ProfessionName = UEnum::GetDisplayValueAsText(Request.Profession).ToString();
		Info.CurrentLevel = Request.Trainee->GetSkillLevel(Info.Profession);
		Info.TargetLevel = Request.TargetLevel;
		Info.Progress = 0.0f;
		Info.TurnsCompleted = 0;
		Info.TotalTurns = Request.TotalTurns;
		Info.bIsActive = false;

		CachedQueue.Add(Info);
	}

	QueueSize = CachedQueue.Num();

	// Notify Blueprint
	OnTrainingQueueUpdated();
}

TArray<FTrainingDisplayInfo> UTrainingQueueWidget::GetTrainingQueue() const
{
	return CachedQueue;
}

FTrainingDisplayInfo UTrainingQueueWidget::GetActiveTraining() const
{
	if (CachedQueue.Num() > 0)
		return CachedQueue[0];

	return FTrainingDisplayInfo();
}

bool UTrainingQueueWidget::StartTraining(ABaseVillager* Villager, EBuildingType Profession, ESkillLevel TargetLevel)
{
	if (!TargetGuildHall)
	{
		UE_LOG(LogTemp, Warning, TEXT("TrainingQueueWidget: No guild hall set"));
		return false;
	}

	bool bSuccess = TargetGuildHall->StartTraining(Villager, Profession, TargetLevel);

	if (bSuccess)
	{
		UpdateTrainingQueue();
	}

	return bSuccess;
}

void UTrainingQueueWidget::CancelTraining()
{
	if (!TargetGuildHall)
		return;

	TargetGuildHall->CancelTraining();
	UpdateTrainingQueue();
}

TMap<EResourceType, int32> UTrainingQueueWidget::GetTrainingCost(ESkillLevel CurrentLevel, ESkillLevel TargetLevel) const
{
	if (!TargetGuildHall)
		return TMap<EResourceType, int32>();

	return TargetGuildHall->GetTrainingCost(CurrentLevel, TargetLevel);
}

int32 UTrainingQueueWidget::GetTrainingDuration(ESkillLevel CurrentLevel, ESkillLevel TargetLevel) const
{
	if (!TargetGuildHall)
		return 0;

	return TargetGuildHall->GetTrainingDuration(CurrentLevel, TargetLevel);
}

bool UTrainingQueueWidget::CanAffordTraining(ESkillLevel CurrentLevel, ESkillLevel TargetLevel) const
{
	if (!TargetGuildHall)
		return false;

	return TargetGuildHall->CanAffordTraining(CurrentLevel, TargetLevel);
}

FString UTrainingQueueWidget::GetTrainingCostText(ESkillLevel CurrentLevel, ESkillLevel TargetLevel) const
{
	TMap<EResourceType, int32> Cost = GetTrainingCost(CurrentLevel, TargetLevel);

	if (Cost.Num() == 0)
		return TEXT("Free");

	FString Result;
	for (const auto& Pair : Cost)
	{
		if (Result.Len() > 0)
			Result += TEXT(", ");

		Result += FString::Printf(TEXT("%d %s"),
			Pair.Value,
			*UEnum::GetDisplayValueAsText(Pair.Key).ToString());
	}

	return Result;
}
