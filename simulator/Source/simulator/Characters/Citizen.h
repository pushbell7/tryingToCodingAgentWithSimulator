// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseVillager.h"
#include "Citizen.generated.h"

UCLASS()
class SIMULATOR_API ACitizen : public ABaseVillager
{
	GENERATED_BODY()

public:
	ACitizen();

protected:
	virtual void BeginPlay() override;

public:
	// Citizen-specific properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Citizen")
	FString Occupation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Citizen")
	bool bIsWorking;
};
