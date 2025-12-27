// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VillageGameMode.generated.h"

UCLASS()
class SIMULATOR_API AVillageGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AVillageGameMode();

protected:
	virtual void BeginPlay() override;

public:
	// Spawn villagers in the world
	UFUNCTION(BlueprintCallable, Category = "Village")
	void SpawnVillagers(int32 NumCitizens, int32 NumGuards, int32 NumMerchants);

	UFUNCTION(BlueprintCallable, Category = "Village")
	class ABaseVillager* SpawnVillagerAtLocation(TSubclassOf<class ABaseVillager> VillagerClass, FVector Location);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Village")
	TSubclassOf<class ACitizen> CitizenClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Village")
	TSubclassOf<class AGuard> GuardClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Village")
	TSubclassOf<class AMerchant> MerchantClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Village")
	float SpawnRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Village")
	int32 InitialCitizens;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Village")
	int32 InitialGuards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Village")
	int32 InitialMerchants;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Village")
	bool bAutoSpawnOnBeginPlay;

private:
	FVector GetRandomSpawnLocation();
};
