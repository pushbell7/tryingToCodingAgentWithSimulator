// Copyright Epic Games, Inc. All Rights Reserved.

#include "Merchant.h"

AMerchant::AMerchant()
{
	VillagerRole = EVillagerRole::Merchant;
	VillagerName = TEXT("Merchant");
	ShopName = TEXT("General Store");
	ShopLocation = FVector::ZeroVector;
	bIsShopOpen = true;
	Gold = 100;
}

void AMerchant::BeginPlay()
{
	Super::BeginPlay();

	// Set shop location to initial location
	ShopLocation = GetActorLocation();
}
