// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseVillager.h"
#include "Merchant.generated.h"

UCLASS()
class SIMULATOR_API AMerchant : public ABaseVillager
{
	GENERATED_BODY()

public:
	AMerchant();

protected:
	virtual void BeginPlay() override;

public:
	// Merchant-specific properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant")
	FString ShopName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant")
	FVector ShopLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant")
	bool bIsShopOpen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant")
	int32 Gold;
};
