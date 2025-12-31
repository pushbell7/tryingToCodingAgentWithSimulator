// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseBuilding.h"
#include "SimulatorTypes.h"
#include "TerritoryLandmark.generated.h"

/**
 * Territory Landmark - Ownership marker building
 * Destroying enemy landmark + building your own = territory conquest
 * HP-based destruction system
 */
UCLASS()
class SIMULATOR_API ATerritoryLandmark : public ABaseBuilding
{
	GENERATED_BODY()

public:
	ATerritoryLandmark();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// === Territory Connection ===
	// Note: OwnerTerritory is inherited from BaseBuilding

	// Set owner territory
	UFUNCTION(BlueprintCallable, Category = "Landmark")
	void SetOwnerTerritory(class ATerritory* Territory);

	// === HP System ===

	// Maximum health points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark|Combat")
	float MaxHealth;

	// Current health points
	UPROPERTY(BlueprintReadOnly, Category = "Landmark|Combat")
	float CurrentHealth;

	// Is the landmark destroyed?
	UPROPERTY(BlueprintReadOnly, Category = "Landmark|Combat")
	bool bIsDestroyed;

	// Take damage from attacks
	UFUNCTION(BlueprintCallable, Category = "Landmark|Combat")
	void TakeLandmarkDamage(float Damage);

	// Repair the landmark (restore HP)
	UFUNCTION(BlueprintCallable, Category = "Landmark|Combat")
	void Repair(float Amount);

	// Get health percentage
	UFUNCTION(BlueprintCallable, Category = "Landmark|Combat")
	float GetHealthPercentage() const;

	// === Construction/Destruction ===

	// Called when landmark is destroyed
	UFUNCTION(BlueprintCallable, Category = "Landmark")
	void OnDestroyed();

	// Called when landmark construction is completed
	UFUNCTION(BlueprintCallable, Category = "Landmark")
	void OnConstructionCompleted();

	// Check if this landmark can be built at this location
	UFUNCTION(BlueprintCallable, Category = "Landmark")
	static bool CanBuildAtLocation(UWorld* World, FVector Location, int32 FactionID);

	// === Visual Feedback ===

	// Update visual state based on health
	UFUNCTION(BlueprintCallable, Category = "Landmark")
	void UpdateVisualState();

	// Particle effect for destruction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark|Effects")
	class UParticleSystem* DestructionEffect;

	// Particle effect for construction completion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark|Effects")
	class UParticleSystem* CompletionEffect;
};
