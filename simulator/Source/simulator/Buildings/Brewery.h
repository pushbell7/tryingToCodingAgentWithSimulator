// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseBuilding.h"
#include "Brewery.generated.h"

/**
 * Brewery - Tier 3 Processing Building
 * Ferments Food into Ale (luxury good)
 * Requires Master skill level
 */
UCLASS()
class SIMULATOR_API ABrewery : public ABaseBuilding
{
	GENERATED_BODY()

public:
	ABrewery();

protected:
	virtual void BeginPlay() override;
};
