// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseBuilding.h"
#include "Sawmill.generated.h"

/**
 * Sawmill - Tier 2 Processing Building
 * Processes Wood into Planks
 * Requires Apprentice skill level
 */
UCLASS()
class SIMULATOR_API ASawmill : public ABaseBuilding
{
	GENERATED_BODY()

public:
	ASawmill();

protected:
	virtual void BeginPlay() override;
};
