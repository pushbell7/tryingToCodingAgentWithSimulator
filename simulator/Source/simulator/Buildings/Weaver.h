// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseBuilding.h"
#include "Weaver.generated.h"

/**
 * Weaver - Tier 3 Processing Building
 * Weaves Cloth from raw materials (luxury good)
 * Requires Master skill level
 * Note: Input resource currently undefined in EResourceType - can be added later
 */
UCLASS()
class SIMULATOR_API AWeaver : public ABaseBuilding
{
	GENERATED_BODY()

public:
	AWeaver();

protected:
	virtual void BeginPlay() override;
};
