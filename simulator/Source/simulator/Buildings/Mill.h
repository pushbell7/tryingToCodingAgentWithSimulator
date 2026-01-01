// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseBuilding.h"
#include "Mill.generated.h"

/**
 * Mill - Tier 2 Processing Building
 * Grinds Food (wheat) into Bread
 * Requires Apprentice skill level
 */
UCLASS()
class SIMULATOR_API AMill : public ABaseBuilding
{
	GENERATED_BODY()

public:
	AMill();

protected:
	virtual void BeginPlay() override;
};
