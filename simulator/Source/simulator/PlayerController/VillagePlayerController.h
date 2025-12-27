// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VillagePlayerController.generated.h"

UCLASS()
class SIMULATOR_API AVillagePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AVillagePlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:
	// Possess a villager by clicking on them
	UFUNCTION(BlueprintCallable, Category = "Possession")
	void PossessVillagerAtLocation(FVector Location);

	// Possess the nearest villager
	UFUNCTION(BlueprintCallable, Category = "Possession")
	void PossessNearestVillager();

	// Unpossess current villager and switch to spectator
	UFUNCTION(BlueprintCallable, Category = "Possession")
	void UnpossessVillager();

	// Find villager under mouse cursor
	UFUNCTION(BlueprintCallable, Category = "Possession")
	class ABaseVillager* GetVillagerUnderCursor();

protected:
	void OnMouseClick();

	UPROPERTY()
	class ABaseVillager* CurrentPossessedVillager;

	UPROPERTY()
	class AAIController* VillagerPreviousController;
};
