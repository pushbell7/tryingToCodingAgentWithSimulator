// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseBuilding.h"
#include "Tannery.generated.h"

/**
 * Tannery - Tier 2 Processing Building
 * Processes Meat into Leather
 * Requires Apprentice skill level
 */
UCLASS()
class SIMULATOR_API ATannery : public ABaseBuilding
{
	GENERATED_BODY()

public:
	ATannery();

protected:
	virtual void BeginPlay() override;
};
