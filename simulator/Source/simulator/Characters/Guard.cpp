// Copyright Epic Games, Inc. All Rights Reserved.

#include "Guard.h"

AGuard::AGuard()
{
	VillagerRole = EVillagerRole::Guard;
	VillagerName = TEXT("Guard");
	DetectionRadius = 1000.0f;
	bIsOnDuty = true;
	GuardPostLocation = FVector::ZeroVector;
}

void AGuard::BeginPlay()
{
	Super::BeginPlay();

	// Set guard post to initial location
	GuardPostLocation = GetActorLocation();
}
