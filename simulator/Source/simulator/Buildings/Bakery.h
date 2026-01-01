// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseBuilding.h"
#include "Bakery.generated.h"

/**
 * Bakery - Tier 2 Processing Building
 * Bakes Food (flour) into Bread
 * Alternative to Mill with higher efficiency
 * Requires Apprentice skill level
 */
UCLASS()
class SIMULATOR_API ABakery : public ABaseBuilding
{
	GENERATED_BODY()

public:
	ABakery();

protected:
	virtual void BeginPlay() override;
};
