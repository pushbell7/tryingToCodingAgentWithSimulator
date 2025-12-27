// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseVillager.h"
#include "Guard.generated.h"

UCLASS()
class SIMULATOR_API AGuard : public ABaseVillager
{
	GENERATED_BODY()

public:
	AGuard();

protected:
	virtual void BeginPlay() override;

public:
	// Guard-specific properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard")
	float DetectionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard")
	bool bIsOnDuty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard")
	FVector GuardPostLocation;
};
