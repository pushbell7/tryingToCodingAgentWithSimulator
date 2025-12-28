// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseVillager.h"
#include "SimulatorTypes.h"
#include "MerchantVillager.generated.h"

/**
 * Trade offer - what merchant is willing to buy/sell
 */
USTRUCT(BlueprintType)
struct FTradeOffer
{
	GENERATED_BODY()

	// Resource being offered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	EResourceType ResourceType;

	// Quantity available
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	int32 Quantity;

	// Price per unit (in gold or other resource)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	int32 PricePerUnit;

	// Is this a buy offer (merchant buying) or sell offer (merchant selling)?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	bool bIsBuyOffer;

	FTradeOffer()
		: ResourceType(EResourceType::Food)
		, Quantity(0)
		, PricePerUnit(1)
		, bIsBuyOffer(false)
	{}

	FTradeOffer(EResourceType InType, int32 InQuantity, int32 InPrice, bool bBuy)
		: ResourceType(InType)
		, Quantity(InQuantity)
		, PricePerUnit(InPrice)
		, bIsBuyOffer(bBuy)
	{}
};

/**
 * Merchant villager specialized in trading resources
 * Buys and sells goods at the market
 */
UCLASS()
class SIMULATOR_API AMerchantVillager : public ABaseVillager
{
	GENERATED_BODY()

public:
	AMerchantVillager();

protected:
	virtual void BeginPlay() override;

public:
	// Available trade offers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant")
	TArray<FTradeOffer> TradeOffers;

	// Merchant's gold reserve
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant")
	int32 GoldReserve;

	// Assigned market building
	UPROPERTY(BlueprintReadWrite, Category = "Merchant")
	class ABaseBuilding* AssignedMarket;

	// Trading profit margin (1.0 = 100% markup)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant")
	float ProfitMargin;

	// Assign merchant to a market
	UFUNCTION(BlueprintCallable, Category = "Merchant")
	bool AssignToMarket(class ABaseBuilding* Market);

	// Unassign from market
	UFUNCTION(BlueprintCallable, Category = "Merchant")
	void UnassignFromMarket();

	// Check if merchant is assigned
	UFUNCTION(BlueprintCallable, Category = "Merchant")
	bool IsAssigned() const;

	// Add a trade offer
	UFUNCTION(BlueprintCallable, Category = "Merchant")
	void AddTradeOffer(FTradeOffer Offer);

	// Remove a trade offer
	UFUNCTION(BlueprintCallable, Category = "Merchant")
	void RemoveTradeOffer(EResourceType ResourceType, bool bIsBuyOffer);

	// Get buy price for a resource
	UFUNCTION(BlueprintCallable, Category = "Merchant")
	int32 GetBuyPrice(EResourceType ResourceType) const;

	// Get sell price for a resource
	UFUNCTION(BlueprintCallable, Category = "Merchant")
	int32 GetSellPrice(EResourceType ResourceType) const;

	// Execute a trade (returns true if successful)
	UFUNCTION(BlueprintCallable, Category = "Merchant")
	bool ExecuteTrade(EResourceType ResourceType, int32 Quantity, bool bPlayerBuying);
};
