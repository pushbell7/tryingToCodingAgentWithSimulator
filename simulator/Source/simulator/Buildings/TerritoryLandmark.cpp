// Copyright Epic Games, Inc. All Rights Reserved.

#include "TerritoryLandmark.h"
#include "Territory.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

ATerritoryLandmark::ATerritoryLandmark()
{
	PrimaryActorTick.bCanEverTick = true;

	// Building info
	BuildingType = EBuildingType::Landmark;
	BuildingName = TEXT("Territory Landmark");

	// HP system
	MaxHealth = 5000.0f;
	CurrentHealth = MaxHealth;
	bIsDestroyed = false;

	// Construction cost
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Wood, 300));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Stone, 500));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Iron, 100));
	ConstructionCost.RequiredWorkAmount = 1000.0f;
	ConstructionCost.MaxWorkers = 10;

	// Territory
	OwnerTerritory = nullptr;

	// Not operational until construction completed
	bIsOperational = false;
}

void ATerritoryLandmark::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("TerritoryLandmark created: %s"), *BuildingName);
}

void ATerritoryLandmark::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update visual state based on health
	UpdateVisualState();
}

void ATerritoryLandmark::SetOwnerTerritory(ATerritory* Territory)
{
	if (Territory)
	{
		OwnerTerritory = Territory;

		UE_LOG(LogTemp, Log, TEXT("Landmark connected to Territory: %s"),
			*Territory->TerritoryName);
	}
}

void ATerritoryLandmark::TakeLandmarkDamage(float Damage)
{
	if (bIsDestroyed) return;

	CurrentHealth -= Damage;

	UE_LOG(LogTemp, Warning, TEXT("Landmark took %.0f damage. HP: %.0f/%.0f"),
		Damage, CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		CurrentHealth = 0.0f;
		OnDestroyed();
	}
}

void ATerritoryLandmark::Repair(float Amount)
{
	if (bIsDestroyed) return;

	CurrentHealth = FMath::Min(CurrentHealth + Amount, MaxHealth);

	UE_LOG(LogTemp, Log, TEXT("Landmark repaired by %.0f. HP: %.0f/%.0f"),
		Amount, CurrentHealth, MaxHealth);
}

float ATerritoryLandmark::GetHealthPercentage() const
{
	if (MaxHealth <= 0.0f) return 0.0f;
	return CurrentHealth / MaxHealth;
}

void ATerritoryLandmark::OnDestroyed()
{
	if (bIsDestroyed) return;

	bIsDestroyed = true;
	bIsOperational = false;

	UE_LOG(LogTemp, Warning, TEXT("Landmark DESTROYED: %s"), *BuildingName);

	// Spawn destruction effect
	if (DestructionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			DestructionEffect,
			GetActorLocation(),
			FRotator::ZeroRotator,
			FVector(2.0f)  // Scale
		);
	}

	// Notify territory of landmark destruction
	if (OwnerTerritory)
	{
		OwnerTerritory->OnLandmarkDestroyed();
	}

	// TODO: Trigger visual change (rubble/ruins)
}

void ATerritoryLandmark::OnConstructionCompleted()
{
	bIsOperational = true;
	bIsDestroyed = false;
	CurrentHealth = MaxHealth;

	UE_LOG(LogTemp, Log, TEXT("Landmark construction COMPLETED: %s"), *BuildingName);

	// Spawn completion effect
	if (CompletionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			CompletionEffect,
			GetActorLocation(),
			FRotator::ZeroRotator,
			FVector(2.0f)
		);
	}

	// Notify territory of landmark completion
	if (OwnerTerritory)
	{
		OwnerTerritory->OnLandmarkCompleted();
	}
}

bool ATerritoryLandmark::CanBuildAtLocation(UWorld* World, FVector Location, int32 FactionID)
{
	if (!World) return false;

	// TODO: Check if location is inside a neutral or enemy territory
	// For now, always allow building
	return true;
}

void ATerritoryLandmark::UpdateVisualState()
{
	// TODO: Update material/mesh based on health percentage
	// Low health = damaged appearance
	// Destroyed = ruins/rubble

	float HealthPercent = GetHealthPercentage();

	if (BuildingMesh)
	{
		// Example: Adjust opacity based on health
		// This would require material parameters in a real implementation
	}
}
