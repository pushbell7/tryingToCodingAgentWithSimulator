// Copyright Epic Games, Inc. All Rights Reserved.

#include "BaseBuilding.h"
#include "InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "BaseVillager.h"

ABaseBuilding::ABaseBuilding()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create mesh component
	BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
	RootComponent = BuildingMesh;

	// Create inventory component
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	if (Inventory)
	{
		Inventory->MaxCapacity = 500; // Buildings can store 500 items by default
	}

	// Default values
	BuildingType = EBuildingType::Warehouse;
	BuildingName = TEXT("Building");
	bIsOperational = true;
	MaxWorkers = 1;
	CurrentWorkers = 0;
}

void ABaseBuilding::BeginPlay()
{
	Super::BeginPlay();

	// Auto-name building based on type
	FString TypeName;
	switch (BuildingType)
	{
	case EBuildingType::Warehouse:
		TypeName = TEXT("Warehouse");
		break;
	case EBuildingType::Granary:
		TypeName = TEXT("Granary");
		break;
	case EBuildingType::Mill:
		TypeName = TEXT("Mill");
		break;
	case EBuildingType::Bakery:
		TypeName = TEXT("Bakery");
		break;
	case EBuildingType::Tannery:
		TypeName = TEXT("Tannery");
		break;
	case EBuildingType::Sawmill:
		TypeName = TEXT("Sawmill");
		break;
	case EBuildingType::Blacksmith:
		TypeName = TEXT("Blacksmith");
		break;
	case EBuildingType::Brewery:
		TypeName = TEXT("Brewery");
		break;
	case EBuildingType::Weaver:
		TypeName = TEXT("Weaver");
		break;
	case EBuildingType::Market:
		TypeName = TEXT("Market");
		break;
	case EBuildingType::House:
		TypeName = TEXT("House");
		break;
	case EBuildingType::TownHall:
		TypeName = TEXT("Town Hall");
		break;
	default:
		TypeName = TEXT("Unknown Building");
		break;
	}

	if (BuildingName == TEXT("Building"))
	{
		BuildingName = TypeName;
	}

	UE_LOG(LogTemp, Log, TEXT("Building '%s' initialized - Type: %s, Operational: %s"),
		*BuildingName, *TypeName, bIsOperational ? TEXT("Yes") : TEXT("No"));
}

bool ABaseBuilding::CanAcceptResources() const
{
	if (!bIsOperational || !Inventory)
	{
		return false;
	}

	// Storage buildings can always accept if not full
	if (IsStorageBuilding())
	{
		return !Inventory->IsFull();
	}

	// Processing buildings need to check if they can process
	return true;
}

bool ABaseBuilding::HasResources() const
{
	if (!Inventory)
	{
		return false;
	}

	return Inventory->GetTotalItems() > 0;
}

FVector ABaseBuilding::GetBuildingLocation() const
{
	return GetActorLocation();
}

bool ABaseBuilding::IsStorageBuilding() const
{
	return BuildingType == EBuildingType::Warehouse ||
		BuildingType == EBuildingType::Granary;
}

bool ABaseBuilding::IsProcessingBuilding() const
{
	return BuildingType == EBuildingType::Mill ||
		BuildingType == EBuildingType::Bakery ||
		BuildingType == EBuildingType::Tannery ||
		BuildingType == EBuildingType::Sawmill ||
		BuildingType == EBuildingType::Blacksmith ||
		BuildingType == EBuildingType::Brewery ||
		BuildingType == EBuildingType::Weaver;
}

bool ABaseBuilding::AddWorker(ABaseVillager* Worker)
{
	if (!Worker)
	{
		UE_LOG(LogTemp, Warning, TEXT("Building '%s': Cannot add null worker"), *BuildingName);
		return false;
	}

	// Check if already assigned
	if (AssignedWorkers.Contains(Worker))
	{
		UE_LOG(LogTemp, Warning, TEXT("Building '%s': Worker %s already assigned"), *BuildingName, *Worker->VillagerName);
		return false;
	}

	// Check capacity
	if (CurrentWorkers >= MaxWorkers)
	{
		UE_LOG(LogTemp, Warning, TEXT("Building '%s': Full capacity (%d/%d)"), *BuildingName, CurrentWorkers, MaxWorkers);
		return false;
	}

	// Add worker
	AssignedWorkers.Add(Worker);
	CurrentWorkers = AssignedWorkers.Num();

	UE_LOG(LogTemp, Log, TEXT("Building '%s': Added worker %s (%d/%d)"),
		*BuildingName, *Worker->VillagerName, CurrentWorkers, MaxWorkers);

	return true;
}

bool ABaseBuilding::RemoveWorker(ABaseVillager* Worker)
{
	if (!Worker)
	{
		return false;
	}

	// Remove from list
	int32 Removed = AssignedWorkers.Remove(Worker);
	if (Removed > 0)
	{
		CurrentWorkers = AssignedWorkers.Num();
		UE_LOG(LogTemp, Log, TEXT("Building '%s': Removed worker %s (%d/%d)"),
			*BuildingName, *Worker->VillagerName, CurrentWorkers, MaxWorkers);
		return true;
	}

	return false;
}

bool ABaseBuilding::HasAvailableWorkerSlots() const
{
	return CurrentWorkers < MaxWorkers;
}
