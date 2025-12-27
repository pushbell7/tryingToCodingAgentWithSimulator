// Copyright Epic Games, Inc. All Rights Reserved.

#include "Citizen.h"

ACitizen::ACitizen()
{
	VillagerRole = EVillagerRole::Citizen;
	VillagerName = TEXT("Citizen");
	Occupation = TEXT("Worker");
	bIsWorking = false;
}

void ACitizen::BeginPlay()
{
	Super::BeginPlay();
}
