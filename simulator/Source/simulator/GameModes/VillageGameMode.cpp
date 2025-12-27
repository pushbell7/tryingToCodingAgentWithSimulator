// Copyright Epic Games, Inc. All Rights Reserved.

#include "VillageGameMode.h"
#include "Citizen.h"
#include "Guard.h"
#include "Merchant.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

AVillageGameMode::AVillageGameMode()
{
	SpawnRadius = 5000.0f;
	InitialCitizens = 5;
	InitialGuards = 2;
	InitialMerchants = 2;
	bAutoSpawnOnBeginPlay = true;

	// Set default classes to the C++ classes
	CitizenClass = ACitizen::StaticClass();
	GuardClass = AGuard::StaticClass();
	MerchantClass = AMerchant::StaticClass();
}

void AVillageGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("VillageGameMode::BeginPlay called!"));

	// Auto-spawn villagers if enabled
	if (bAutoSpawnOnBeginPlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("Auto-spawn is enabled. Spawning %d citizens, %d guards, %d merchants"), InitialCitizens, InitialGuards, InitialMerchants);

		// Small delay to ensure world is ready
		FTimerHandle SpawnTimerHandle;
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, [this]()
		{
			SpawnVillagers(InitialCitizens, InitialGuards, InitialMerchants);
		}, 0.5f, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Auto-spawn is DISABLED"));
	}
}

void AVillageGameMode::SpawnVillagers(int32 NumCitizens, int32 NumGuards, int32 NumMerchants)
{
	UE_LOG(LogTemp, Warning, TEXT("SpawnVillagers called!"));

	// Spawn Citizens
	for (int32 i = 0; i < NumCitizens; i++)
	{
		FVector SpawnLocation = GetRandomSpawnLocation();
		UE_LOG(LogTemp, Warning, TEXT("Spawning Citizen %d at location: %s"), i, *SpawnLocation.ToString());
		if (CitizenClass)
		{
			ABaseVillager* Spawned = SpawnVillagerAtLocation(CitizenClass, SpawnLocation);
			if (Spawned)
			{
				UE_LOG(LogTemp, Warning, TEXT("Citizen %d spawned successfully!"), i);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to spawn Citizen %d"), i);
			}
		}
	}

	// Spawn Guards
	for (int32 i = 0; i < NumGuards; i++)
	{
		FVector SpawnLocation = GetRandomSpawnLocation();
		if (GuardClass)
		{
			SpawnVillagerAtLocation(GuardClass, SpawnLocation);
		}
	}

	// Spawn Merchants
	for (int32 i = 0; i < NumMerchants; i++)
	{
		FVector SpawnLocation = GetRandomSpawnLocation();
		if (MerchantClass)
		{
			SpawnVillagerAtLocation(MerchantClass, SpawnLocation);
		}
	}
}

ABaseVillager* AVillageGameMode::SpawnVillagerAtLocation(TSubclassOf<ABaseVillager> VillagerClass, FVector Location)
{
	if (!VillagerClass)
		return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ABaseVillager* SpawnedVillager = GetWorld()->SpawnActor<ABaseVillager>(VillagerClass, Location, FRotator::ZeroRotator, SpawnParams);

	return SpawnedVillager;
}

FVector AVillageGameMode::GetRandomSpawnLocation()
{
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	FVector Origin = FVector::ZeroVector;

	// Try to get player start location as origin
	AActor* PlayerStart = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());
	if (PlayerStart)
	{
		Origin = PlayerStart->GetActorLocation();
	}

	if (NavSystem)
	{
		FNavLocation RandomLocation;
		bool bFound = NavSystem->GetRandomPointInNavigableRadius(Origin, SpawnRadius, RandomLocation);
		if (bFound)
		{
			return RandomLocation.Location;
		}
	}

	// Fallback to random location around origin
	float RandomX = FMath::RandRange(-SpawnRadius, SpawnRadius);
	float RandomY = FMath::RandRange(-SpawnRadius, SpawnRadius);
	return Origin + FVector(RandomX, RandomY, 0.0f);
}
