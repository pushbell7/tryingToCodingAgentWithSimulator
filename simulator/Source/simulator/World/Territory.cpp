// Copyright Epic Games, Inc. All Rights Reserved.

#include "Territory.h"
#include "BaseBuilding.h"
#include "TradingPost.h"
#include "TerritoryLandmark.h"
#include "BaseVillager.h"
#include "Caravan.h"
#include "TurnManagerSubsystem.h"
#include "GuildHall.h"
#include "Kismet/GameplayStatics.h"

ATerritory::ATerritory()
{
	PrimaryActorTick.bCanEverTick = true;

	// 기본 정보
	TerritoryName = TEXT("New Territory");
	OwnerFactionID = 0;  // 0 = neutral
	TerritoryCenter = FVector::ZeroVector;
	TerritoryRadius = 5000.0f; // 5000 units 반경

	// 상태
	TerritoryState = ETerritoryState::Neutral;
	NeutralStateDuration = 0.0f;

	// 자원
	MaxStorageCapacity = 10000; // 대용량 중앙 창고

	// 건물/주민
	TradingPost = nullptr;
	Landmark = nullptr;

	// 중립 상태 감쇠율
	NeutralResourceDecayRate = 0.1f;  // 초당 0.1 자원 감소
	NeutralPopulationDecayRate = 0.01f;  // 초당 0.01 인구 감소 (매우 느림)
}

void ATerritory::BeginPlay()
{
	Super::BeginPlay();

	TerritoryCenter = GetActorLocation();

	// Register with TurnManager for automatic turn processing
	if (UWorld* World = GetWorld())
	{
		if (UTurnManagerSubsystem* TurnManager = World->GetSubsystem<UTurnManagerSubsystem>())
		{
			TurnManager->RegisterTerritory(this);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Territory %s created (Faction: %d, Radius: %.0f)"),
		*TerritoryName, OwnerFactionID, TerritoryRadius);
}

void ATerritory::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister from TurnManager
	if (UWorld* World = GetWorld())
	{
		if (UTurnManagerSubsystem* TurnManager = World->GetSubsystem<UTurnManagerSubsystem>())
		{
			TurnManager->UnregisterTerritory(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ATerritory::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Process neutral state decay
	if (TerritoryState == ETerritoryState::Neutral)
	{
		NeutralStateDuration += DeltaTime;
		ProcessNeutralDecay(DeltaTime);
	}
}

int32 ATerritory::GetTotalResourceAmount() const
{
	int32 Total = 0;
	for (const auto& Pair : TerritoryResources)
	{
		Total += Pair.Value;
	}
	return Total;
}

bool ATerritory::AddResource(EResourceType ResourceType, int32 Amount)
{
	if (Amount <= 0) return false;

	// 용량 체크
	int32 CurrentAmount = GetTotalResourceAmount();
	if (CurrentAmount + Amount > MaxStorageCapacity)
	{
		UE_LOG(LogTemp, Warning, TEXT("Territory %s: Storage capacity exceeded"),
			*TerritoryName);
		return false;
	}

	// 자원 추가
	if (!TerritoryResources.Contains(ResourceType))
	{
		TerritoryResources.Add(ResourceType, 0);
	}

	TerritoryResources[ResourceType] += Amount;

	UE_LOG(LogTemp, Log, TEXT("Territory %s: +%d %s (Total: %d)"),
		*TerritoryName, Amount,
		*UEnum::GetValueAsString(ResourceType),
		TerritoryResources[ResourceType]);

	return true;
}

bool ATerritory::RemoveResource(EResourceType ResourceType, int32 Amount)
{
	if (Amount <= 0) return false;

	if (!TerritoryResources.Contains(ResourceType) || TerritoryResources[ResourceType] < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("Territory %s: Not enough %s to remove"),
			*TerritoryName, *UEnum::GetValueAsString(ResourceType));
		return false;
	}

	TerritoryResources[ResourceType] -= Amount;

	UE_LOG(LogTemp, Log, TEXT("Territory %s: -%d %s (Remaining: %d)"),
		*TerritoryName, Amount,
		*UEnum::GetValueAsString(ResourceType),
		TerritoryResources[ResourceType]);

	return true;
}

int32 ATerritory::GetResourceAmount(EResourceType ResourceType) const
{
	if (TerritoryResources.Contains(ResourceType))
	{
		return TerritoryResources[ResourceType];
	}
	return 0;
}

bool ATerritory::HasResource(EResourceType ResourceType, int32 Amount) const
{
	return GetResourceAmount(ResourceType) >= Amount;
}

void ATerritory::RegisterBuilding(ABaseBuilding* Building)
{
	if (Building && !Buildings.Contains(Building))
	{
		Buildings.Add(Building);

		// Set building's owner territory reference (for resource access)
		Building->OwnerTerritory = this;

		// 교역소 자동 감지
		ATradingPost* Post = Cast<ATradingPost>(Building);
		if (Post)
		{
			SetTradingPost(Post);
		}

		// 랜드마크 자동 감지
		ATerritoryLandmark* LandmarkBuilding = Cast<ATerritoryLandmark>(Building);
		if (LandmarkBuilding)
		{
			SetLandmark(LandmarkBuilding);
		}

		UE_LOG(LogTemp, Log, TEXT("Territory %s: Building %s registered"),
			*TerritoryName, *Building->BuildingName);
	}
}

void ATerritory::UnregisterBuilding(ABaseBuilding* Building)
{
	if (Building)
	{
		Buildings.Remove(Building);

		if (TradingPost == Building)
		{
			TradingPost = nullptr;
		}

		if (Landmark == Building)
		{
			Landmark = nullptr;
		}
	}
}

void ATerritory::SetTradingPost(ATradingPost* Post)
{
	if (Post)
	{
		TradingPost = Post;
		Post->TerritoryName = TerritoryName;
		Post->OwnerFactionID = OwnerFactionID;

		UE_LOG(LogTemp, Log, TEXT("Territory %s: Trading Post connected"), *TerritoryName);
	}
}

void ATerritory::SetLandmark(ATerritoryLandmark* NewLandmark)
{
	if (NewLandmark)
	{
		Landmark = NewLandmark;
		NewLandmark->SetOwnerTerritory(this);

		UE_LOG(LogTemp, Log, TEXT("Territory %s: Landmark connected"), *TerritoryName);
	}
}

void ATerritory::RegisterVillager(ABaseVillager* Villager)
{
	if (Villager && !Villagers.Contains(Villager))
	{
		Villagers.Add(Villager);

		UE_LOG(LogTemp, Log, TEXT("Territory %s: Villager %s registered (Population: %d)"),
			*TerritoryName, *Villager->VillagerName, GetPopulation());
	}
}

void ATerritory::UnregisterVillager(ABaseVillager* Villager)
{
	Villagers.Remove(Villager);
}

void ATerritory::CalculateProduction()
{
	ProductionPerTurn.Empty();

	if (TerritoryState != ETerritoryState::Owned)
	{
		// Only owned territories produce
		return;
	}

	// Aggregate production from all buildings
	for (ABaseBuilding* Building : Buildings)
	{
		if (!Building || !Building->bIsOperational || !Building->bCanProduce)
			continue;

		// Get production from this building
		TMap<EResourceType, int32> BuildingProduction = Building->CalculateProduction();

		// Add to territory total
		for (const auto& Pair : BuildingProduction)
		{
			if (!ProductionPerTurn.Contains(Pair.Key))
			{
				ProductionPerTurn.Add(Pair.Key, 0);
			}
			ProductionPerTurn[Pair.Key] += Pair.Value;
		}

		// Log individual building production
		if (BuildingProduction.Num() > 0)
		{
			float Efficiency = Building->CalculateLaborEfficiency();
			UE_LOG(LogTemp, Log, TEXT("  %s (Workers: %d/%d, Efficiency: %.0f%%) produces:"),
				*Building->BuildingName, Building->CurrentWorkers, Building->OptimalWorkerCount, Efficiency * 100.0f);

			for (const auto& Pair : BuildingProduction)
			{
				UE_LOG(LogTemp, Log, TEXT("    - %s: %d"),
					*UEnum::GetValueAsString(Pair.Key), Pair.Value);
			}
		}
	}

	// Log total production
	if (ProductionPerTurn.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Territory %s: Total production this turn:"), *TerritoryName);
		for (const auto& Pair : ProductionPerTurn)
		{
			UE_LOG(LogTemp, Log, TEXT("  - %s: %d"),
				*UEnum::GetValueAsString(Pair.Key), Pair.Value);
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Territory %s: No production this turn"), *TerritoryName);
	}
}

void ATerritory::CalculateConsumption()
{
	ConsumptionPerTurn.Empty();

	// 주민 1명당 식량 1 소비
	int32 FoodConsumption = GetPopulation();
	if (FoodConsumption > 0)
	{
		ConsumptionPerTurn.Add(EResourceType::Food, FoodConsumption);
	}

	UE_LOG(LogTemp, Log, TEXT("Territory %s: Consumption calculated (Food: %d)"),
		*TerritoryName, FoodConsumption);
}

void ATerritory::ProcessTurn()
{
	UE_LOG(LogTemp, Log, TEXT("=== Territory %s: Processing Turn ==="), *TerritoryName);

	// 1. 생산/소비 계산
	CalculateProduction();
	CalculateConsumption();

	// 2. 생산 적용
	for (const auto& Pair : ProductionPerTurn)
	{
		AddResource(Pair.Key, Pair.Value);
	}

	// 3. 소비 적용
	for (const auto& Pair : ConsumptionPerTurn)
	{
		if (!RemoveResource(Pair.Key, Pair.Value))
		{
			// 자원 부족 시 경고
			UE_LOG(LogTemp, Warning, TEXT("Territory %s: Insufficient %s for consumption!"),
				*TerritoryName, *UEnum::GetValueAsString(Pair.Key));
		}
	}

	// 4. Process guild hall training (if any)
	for (ABaseBuilding* Building : Buildings)
	{
		AGuildHall* Guild = Cast<AGuildHall>(Building);
		if (Guild && Guild->bIsTraining)
		{
			Guild->ProcessTrainingTurn();
		}
	}

	// 5. 자원 상태 로그
	UE_LOG(LogTemp, Log, TEXT("Territory %s: Resources after turn:"), *TerritoryName);
	for (const auto& Pair : TerritoryResources)
	{
		if (Pair.Value > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("  - %s: %d"),
				*UEnum::GetValueAsString(Pair.Key), Pair.Value);
		}
	}
}

ACaravan* ATerritory::ExportResources(
	ATerritory* Destination,
	TMap<EResourceType, int32> Resources,
	int32 GuardCount)
{
	if (!Destination)
	{
		UE_LOG(LogTemp, Warning, TEXT("Territory %s: Cannot export - invalid destination"),
			*TerritoryName);
		return nullptr;
	}

	if (!TradingPost)
	{
		UE_LOG(LogTemp, Warning, TEXT("Territory %s: Cannot export - no trading post"),
			*TerritoryName);
		return nullptr;
	}

	if (!Destination->TradingPost)
	{
		UE_LOG(LogTemp, Warning, TEXT("Territory %s: Cannot export - destination has no trading post"),
			*TerritoryName);
		return nullptr;
	}

	// 자원 확인 및 인출
	for (const auto& Pair : Resources)
	{
		if (!HasResource(Pair.Key, Pair.Value))
		{
			UE_LOG(LogTemp, Warning, TEXT("Territory %s: Not enough %s to export"),
				*TerritoryName, *UEnum::GetValueAsString(Pair.Key));
			return nullptr;
		}
	}

	// 영지에서 자원 제거
	for (const auto& Pair : Resources)
	{
		RemoveResource(Pair.Key, Pair.Value);
	}

	// 교역소를 통해 상단 파견
	ACaravan* Caravan = TradingPost->SendCaravan(
		Destination->TradingPost,
		Resources,
		GuardCount
	);

	if (Caravan)
	{
		UE_LOG(LogTemp, Log, TEXT("Territory %s -> %s: Caravan dispatched"),
			*TerritoryName, *Destination->TerritoryName);
	}

	return Caravan;
}

void ATerritory::ImportResources(TMap<EResourceType, int32> Resources)
{
	for (const auto& Pair : Resources)
	{
		AddResource(Pair.Key, Pair.Value);
	}

	UE_LOG(LogTemp, Log, TEXT("Territory %s: Resources imported"), *TerritoryName);
}

bool ATerritory::IsActorInTerritory(AActor* Actor) const
{
	if (!Actor) return false;
	return IsLocationInTerritory(Actor->GetActorLocation());
}

bool ATerritory::IsLocationInTerritory(FVector Location) const
{
	float Distance = FVector::Dist(TerritoryCenter, Location);
	return Distance <= TerritoryRadius;
}

float ATerritory::GetDistanceToTerritory(ATerritory* Other) const
{
	if (!Other) return -1.0f;
	return FVector::Dist(TerritoryCenter, Other->TerritoryCenter);
}

void ATerritory::OnLandmarkDestroyed()
{
	UE_LOG(LogTemp, Warning, TEXT("Territory %s: Landmark DESTROYED - Territory becoming neutral"),
		*TerritoryName);

	// Territory becomes neutral when landmark is destroyed
	MakeNeutral();
}

void ATerritory::OnLandmarkCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("Territory %s: Landmark COMPLETED - Territory claimed by faction %d"),
		*TerritoryName, OwnerFactionID);

	// Territory is now owned
	TerritoryState = ETerritoryState::Owned;
	NeutralStateDuration = 0.0f;

	// Transfer remaining resources to new owner (already in territory storage)
	UE_LOG(LogTemp, Log, TEXT("Territory %s: Resources transferred to new owner"), *TerritoryName);
}

void ATerritory::ProcessNeutralDecay(float DeltaTime)
{
	if (TerritoryState != ETerritoryState::Neutral)
		return;

	// Decay resources
	for (auto& Pair : TerritoryResources)
	{
		if (Pair.Value > 0)
		{
			int32 Decay = FMath::RoundToInt(NeutralResourceDecayRate * DeltaTime);
			Pair.Value = FMath::Max(0, Pair.Value - Decay);
		}
	}

	// Decay population (villagers gradually leave)
	if (Villagers.Num() > 0)
	{
		float PopDecay = NeutralPopulationDecayRate * DeltaTime;
		if (FMath::FRand() < PopDecay)
		{
			// Randomly remove a villager
			int32 RandomIndex = FMath::RandRange(0, Villagers.Num() - 1);
			ABaseVillager* VillagerToRemove = Villagers[RandomIndex];

			if (VillagerToRemove)
			{
				UE_LOG(LogTemp, Log, TEXT("Territory %s: Villager %s left due to neutral state"),
					*TerritoryName, *VillagerToRemove->VillagerName);

				// TODO: Actually remove/destroy the villager actor
				Villagers.RemoveAt(RandomIndex);
			}
		}
	}

	// Log decay status periodically (every 60 seconds)
	if (FMath::Fmod(NeutralStateDuration, 60.0f) < DeltaTime)
	{
		UE_LOG(LogTemp, Log, TEXT("Territory %s neutral for %.0f seconds - Resources: %d, Population: %d"),
			*TerritoryName, NeutralStateDuration, GetTotalResourceAmount(), Villagers.Num());
	}
}

void ATerritory::MakeNeutral()
{
	TerritoryState = ETerritoryState::Neutral;
	OwnerFactionID = 0;  // No owner
	NeutralStateDuration = 0.0f;

	// Update trading post if exists
	if (TradingPost)
	{
		TradingPost->OwnerFactionID = 0;
	}

	UE_LOG(LogTemp, Warning, TEXT("Territory %s is now NEUTRAL - resources and population will decay"),
		*TerritoryName);
}

void ATerritory::SetTerritoryOwner(int32 NewFactionID)
{
	OwnerFactionID = NewFactionID;
	TerritoryState = ETerritoryState::Owned;
	NeutralStateDuration = 0.0f;

	// Update trading post if exists
	if (TradingPost)
	{
		TradingPost->OwnerFactionID = NewFactionID;
	}

	UE_LOG(LogTemp, Log, TEXT("Territory %s claimed by faction %d"),
		*TerritoryName, NewFactionID);
}
