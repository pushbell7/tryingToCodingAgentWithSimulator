// Copyright Epic Games, Inc. All Rights Reserved.

#include "MerchantVillager.h"
#include "BaseBuilding.h"
#include "InventoryComponent.h"

AMerchantVillager::AMerchantVillager()
{
	// Merchants have higher social class
	SocialClass = ESocialClass::Merchant;
	VillagerRole = EVillagerRole::Merchant;

	// Default values
	GoldReserve = 1000;
	AssignedMarket = nullptr;
	ProfitMargin = 0.5f; // 50% markup

	// Merchants carry larger inventory for trading
	if (Inventory)
	{
		Inventory->MaxCapacity = 100;
	}

	// Setup default trade offers
	// Buying raw materials
	TradeOffers.Add(FTradeOffer(EResourceType::Food, 50, 2, true));
	TradeOffers.Add(FTradeOffer(EResourceType::Wood, 50, 3, true));
	TradeOffers.Add(FTradeOffer(EResourceType::Stone, 50, 4, true));

	// Selling processed goods
	TradeOffers.Add(FTradeOffer(EResourceType::Bread, 20, 5, false));
	TradeOffers.Add(FTradeOffer(EResourceType::Tools, 10, 15, false));
}

void AMerchantVillager::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("Merchant '%s' initialized - Gold: %d, Margin: %.2f, Offers: %d"),
		*VillagerName, GoldReserve, ProfitMargin, TradeOffers.Num());
}

bool AMerchantVillager::AssignToMarket(ABaseBuilding* Market)
{
	if (!Market)
	{
		UE_LOG(LogTemp, Warning, TEXT("Merchant '%s': Cannot assign to null market"), *VillagerName);
		return false;
	}

	if (Market->BuildingType != EBuildingType::Market)
	{
		UE_LOG(LogTemp, Warning, TEXT("Merchant '%s': Building '%s' is not a market"),
			*VillagerName, *Market->BuildingName);
		return false;
	}

	// Unassign from previous market if any
	if (AssignedMarket)
	{
		UnassignFromMarket();
	}

	AssignedMarket = Market;

	UE_LOG(LogTemp, Log, TEXT("Merchant '%s' assigned to market '%s'"),
		*VillagerName, *Market->BuildingName);

	return true;
}

void AMerchantVillager::UnassignFromMarket()
{
	if (AssignedMarket)
	{
		UE_LOG(LogTemp, Log, TEXT("Merchant '%s' unassigned from market '%s'"),
			*VillagerName, *AssignedMarket->BuildingName);

		AssignedMarket = nullptr;
	}
}

bool AMerchantVillager::IsAssigned() const
{
	return AssignedMarket != nullptr;
}

void AMerchantVillager::AddTradeOffer(FTradeOffer Offer)
{
	TradeOffers.Add(Offer);

	UE_LOG(LogTemp, Log, TEXT("Merchant '%s': Added %s offer for %d x %d at %d gold each"),
		*VillagerName,
		Offer.bIsBuyOffer ? TEXT("BUY") : TEXT("SELL"),
		(int32)Offer.ResourceType,
		Offer.Quantity,
		Offer.PricePerUnit);
}

void AMerchantVillager::RemoveTradeOffer(EResourceType ResourceType, bool bIsBuyOffer)
{
	for (int32 i = TradeOffers.Num() - 1; i >= 0; i--)
	{
		if (TradeOffers[i].ResourceType == ResourceType && TradeOffers[i].bIsBuyOffer == bIsBuyOffer)
		{
			TradeOffers.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("Merchant '%s': Removed %s offer for %d"),
				*VillagerName,
				bIsBuyOffer ? TEXT("BUY") : TEXT("SELL"),
				(int32)ResourceType);
			break;
		}
	}
}

int32 AMerchantVillager::GetBuyPrice(EResourceType ResourceType) const
{
	// Price the player pays to buy from merchant (sell offers)
	for (const FTradeOffer& Offer : TradeOffers)
	{
		if (Offer.ResourceType == ResourceType && !Offer.bIsBuyOffer)
		{
			return Offer.PricePerUnit;
		}
	}
	return 0; // Not available
}

int32 AMerchantVillager::GetSellPrice(EResourceType ResourceType) const
{
	// Price the player receives when selling to merchant (buy offers)
	for (const FTradeOffer& Offer : TradeOffers)
	{
		if (Offer.ResourceType == ResourceType && Offer.bIsBuyOffer)
		{
			return Offer.PricePerUnit;
		}
	}
	return 0; // Not buying
}

bool AMerchantVillager::ExecuteTrade(EResourceType ResourceType, int32 Quantity, bool bPlayerBuying)
{
	if (Quantity <= 0)
	{
		return false;
	}

	// Find matching offer
	FTradeOffer* MatchingOffer = nullptr;
	for (FTradeOffer& Offer : TradeOffers)
	{
		if (Offer.ResourceType == ResourceType && Offer.bIsBuyOffer != bPlayerBuying)
		{
			MatchingOffer = &Offer;
			break;
		}
	}

	if (!MatchingOffer)
	{
		UE_LOG(LogTemp, Warning, TEXT("Merchant '%s': No matching offer for trade"), *VillagerName);
		return false;
	}

	if (Quantity > MatchingOffer->Quantity)
	{
		UE_LOG(LogTemp, Warning, TEXT("Merchant '%s': Insufficient quantity (requested %d, available %d)"),
			*VillagerName, Quantity, MatchingOffer->Quantity);
		return false;
	}

	int32 TotalCost = Quantity * MatchingOffer->PricePerUnit;

	if (bPlayerBuying)
	{
		// Player buying from merchant
		UE_LOG(LogTemp, Log, TEXT("Merchant '%s': Sold %d x %d for %d gold"),
			*VillagerName, (int32)ResourceType, Quantity, TotalCost);
	}
	else
	{
		// Player selling to merchant
		if (GoldReserve < TotalCost)
		{
			UE_LOG(LogTemp, Warning, TEXT("Merchant '%s': Insufficient gold (%d < %d)"),
				*VillagerName, GoldReserve, TotalCost);
			return false;
		}

		GoldReserve -= TotalCost;
		UE_LOG(LogTemp, Log, TEXT("Merchant '%s': Bought %d x %d for %d gold (Reserve: %d)"),
			*VillagerName, (int32)ResourceType, Quantity, TotalCost, GoldReserve);
	}

	// Update offer quantity
	MatchingOffer->Quantity -= Quantity;

	return true;
}
