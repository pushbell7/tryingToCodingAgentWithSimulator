// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseBuilding.h"
#include "Blacksmith.generated.h"

/**
 * Blacksmith - Tier 2 Processing Building
 * Forges Iron and Wood into Tools and Weapons
 * Requires Journeyman skill level for high-quality crafts
 */
UCLASS()
class SIMULATOR_API ABlacksmith : public ABaseBuilding
{
	GENERATED_BODY()

public:
	ABlacksmith();

protected:
	virtual void BeginPlay() override;
};
