// Copyright Epic Games, Inc. All Rights Reserved.

#include "BaseBuilding.h"
#include "InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "BaseVillager.h"
#include "Territory.h"

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

	// Production defaults
	bCanProduce = false;
	OptimalWorkerCount = 3;
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

float ABaseBuilding::CalculateLaborEfficiency() const
{
	if (OptimalWorkerCount <= 0 || CurrentWorkers <= 0)
		return 0.0f;

	if (CurrentWorkers <= OptimalWorkerCount)
	{
		// Under optimal: efficiency scales linearly from 0 to 100%
		// 1 worker / 3 optimal = 33%, 2/3 = 66%, 3/3 = 100%
		return (float)CurrentWorkers / (float)OptimalWorkerCount;
	}
	else
	{
		// Over optimal: diminishing returns
		// 4 workers / 3 optimal = overcrowding penalty
		// Formula: 1.0 - ((excess / optimal) * 0.2)
		// 4/3 = 1.33 -> excess = 1 -> 1.0 - (1/3 * 0.2) = 0.93 (93%)
		// 6/3 = 2.0 -> excess = 3 -> 1.0 - (3/3 * 0.2) = 0.8 (80%)
		int32 Excess = CurrentWorkers - OptimalWorkerCount;
		float Penalty = ((float)Excess / (float)OptimalWorkerCount) * 0.2f;
		return FMath::Max(0.5f, 1.0f - Penalty); // Minimum 50% efficiency
	}
}

bool ABaseBuilding::HasInputResources() const
{
	// No input resources needed = always can produce (Tier 1)
	if (ProductionRecipe.InputResources.Num() == 0)
		return true;

	// No territory = can't check resources
	if (!OwnerTerritory)
		return false;

	// Check if territory has all required input resources
	for (const FResourceStack& Input : ProductionRecipe.InputResources)
	{
		if (!OwnerTerritory->HasResource(Input.ResourceType, Input.Quantity))
		{
			return false;
		}
	}

	return true;
}

TMap<EResourceType, int32> ABaseBuilding::CalculateProduction()
{
	TMap<EResourceType, int32> Production;

	// Can't produce if not operational or no recipe
	if (!bIsOperational || !bCanProduce)
		return Production;

	// No workers = no production
	if (CurrentWorkers <= 0)
		return Production;

	// Check if we have input resources (Tier 2/3 buildings)
	if (!HasInputResources())
	{
		// Log why production stopped
		if (ProductionRecipe.InputResources.Num() > 0)
		{
			UE_LOG(LogTemp, Verbose, TEXT("%s: Production halted - insufficient input resources"), *BuildingName);
		}
		return Production;
	}

	// Consume input resources from territory warehouse
	if (OwnerTerritory && ProductionRecipe.InputResources.Num() > 0)
	{
		for (const FResourceStack& Input : ProductionRecipe.InputResources)
		{
			OwnerTerritory->RemoveResource(Input.ResourceType, Input.Quantity);
		}
	}

	// Calculate efficiency
	float Efficiency = CalculateLaborEfficiency();

	// Apply efficiency to output resources
	for (const FResourceStack& Output : ProductionRecipe.OutputResources)
	{
		int32 BaseProduction = Output.Quantity;
		int32 ActualProduction = FMath::RoundToInt(BaseProduction * Efficiency);

		if (ActualProduction > 0)
		{
			Production.Add(Output.ResourceType, ActualProduction);
		}
	}

	return Production;
}
