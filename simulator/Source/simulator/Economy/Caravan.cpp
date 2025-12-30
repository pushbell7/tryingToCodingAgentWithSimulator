// Copyright Epic Games, Inc. All Rights Reserved.

#include "Caravan.h"
#include "TradingPost.h"
#include "MilitaryUnit.h"
#include "CombatEncounter.h"

ACaravan::ACaravan()
{
	PrimaryActorTick.bCanEverTick = true;

	// 상단 상태
	CaravanState = ECaravanState::Idle;
	OriginTradingPost = nullptr;
	DestinationTradingPost = nullptr;
	OwnerFactionID = 0;

	// 화물
	MaxCargoCapacity = 500;

	// 호위
	GuardUnit = nullptr;
	GuardCount = 0;

	// 이동
	MovementSpeed = 300.0f; // Unreal units/sec
	TravelProgress = 0.0f;

	// 전투
	bIsInCombat = false;
	CurrentCombat = nullptr;
}

void ACaravan::BeginPlay()
{
	Super::BeginPlay();

	CurrentLocation = GetActorLocation();
}

void ACaravan::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 출발지에서 상단 등록 해제
	if (OriginTradingPost)
	{
		OriginTradingPost->UnregisterCaravan(this);
	}

	// 호위 부대 해제
	if (GuardUnit)
	{
		ReleaseGuardUnit();
	}

	Super::EndPlay(EndPlayReason);
}

void ACaravan::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 이동 중이고 전투 중이 아니면 이동 처리
	if (CaravanState == ECaravanState::Traveling && !bIsInCombat)
	{
		UpdateMovement(DeltaTime);
	}
}

int32 ACaravan::GetCurrentCargoAmount() const
{
	int32 Total = 0;
	for (const auto& Pair : CargoResources)
	{
		Total += Pair.Value;
	}
	return Total;
}

bool ACaravan::AddCargo(EResourceType ResourceType, int32 Amount)
{
	if (Amount <= 0) return false;

	int32 CurrentAmount = GetCurrentCargoAmount();
	if (CurrentAmount + Amount > MaxCargoCapacity)
	{
		UE_LOG(LogTemp, Warning, TEXT("Caravan: Cargo capacity exceeded"));
		return false;
	}

	if (!CargoResources.Contains(ResourceType))
	{
		CargoResources.Add(ResourceType, 0);
	}

	CargoResources[ResourceType] += Amount;
	return true;
}

bool ACaravan::RemoveCargo(EResourceType ResourceType, int32 Amount)
{
	if (!CargoResources.Contains(ResourceType) || CargoResources[ResourceType] < Amount)
	{
		return false;
	}

	CargoResources[ResourceType] -= Amount;

	if (CargoResources[ResourceType] == 0)
	{
		CargoResources.Remove(ResourceType);
	}

	return true;
}

TMap<EResourceType, int32> ACaravan::ExtractAllCargo()
{
	TMap<EResourceType, int32> ExtractedCargo = CargoResources;
	CargoResources.Empty();
	return ExtractedCargo;
}

void ACaravan::AssignGuardUnit(AMilitaryUnit* Unit)
{
	if (!Unit) return;

	if (GuardUnit && GuardUnit != Unit)
	{
		ReleaseGuardUnit();
	}

	GuardUnit = Unit;
	GuardCount = Unit->GetUnitSize();

	UE_LOG(LogTemp, Log, TEXT("Caravan assigned guard unit with %d soldiers"), GuardCount);
}

void ACaravan::ReleaseGuardUnit()
{
	if (GuardUnit)
	{
		// 호위 부대를 자유롭게 만듦
		GuardUnit = nullptr;
		GuardCount = 0;

		UE_LOG(LogTemp, Log, TEXT("Caravan released guard unit"));
	}
}

void ACaravan::UpdateMovement(float DeltaTime)
{
	if (!DestinationTradingPost) return;

	float Distance = FVector::Dist(CurrentLocation, TargetLocation);

	if (Distance < 50.0f) // 도착 임계값
	{
		ArrivedAtDestination();
		return;
	}

	// 이동
	FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
	FVector NewLocation = CurrentLocation + (Direction * MovementSpeed * DeltaTime);

	CurrentLocation = NewLocation;
	SetActorLocation(NewLocation);

	// 진행률 업데이트
	float TotalDistance = FVector::Dist(
		OriginTradingPost ? OriginTradingPost->GetActorLocation() : GetActorLocation(),
		TargetLocation
	);

	if (TotalDistance > 0.0f)
	{
		float TraveledDistance = FVector::Dist(
			OriginTradingPost ? OriginTradingPost->GetActorLocation() : GetActorLocation(),
			CurrentLocation
		);
		TravelProgress = FMath::Clamp(TraveledDistance / TotalDistance, 0.0f, 1.0f);
	}

	// 호위 부대도 함께 이동
	if (GuardUnit)
	{
		GuardUnit->SetActorLocation(NewLocation);
	}
}

void ACaravan::EnterCombat(ACombatEncounter* Combat)
{
	if (!Combat) return;

	bIsInCombat = true;
	CurrentCombat = Combat;
	CaravanState = ECaravanState::InCombat;

	UE_LOG(LogTemp, Warning, TEXT("Caravan entered combat!"));

	// 호위 부대가 있으면 전투에 참여
	if (GuardUnit && !GuardUnit->bIsInCombat)
	{
		GuardUnit->EnterCombat(Combat);
	}
}

void ACaravan::LeaveCombat(bool bVictory)
{
	bIsInCombat = false;
	CurrentCombat = nullptr;

	if (bVictory)
	{
		UE_LOG(LogTemp, Log, TEXT("Caravan survived the attack!"));

		// 전투에서 승리했으므로 여행 재개
		if (GetCurrentCargoAmount() > 0)
		{
			CaravanState = ECaravanState::Traveling;
		}
		else
		{
			// 화물을 모두 잃었으면 파괴
			DestroyCaravan();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Caravan defeated! Resources looted."));

		// 패배 시 파괴
		DestroyCaravan();
	}
}

TMap<EResourceType, int32> ACaravan::GetLooted(float LootPercentage)
{
	TMap<EResourceType, int32> LootedResources;

	LootPercentage = FMath::Clamp(LootPercentage, 0.0f, 1.0f);

	for (auto& Pair : CargoResources)
	{
		int32 LootAmount = FMath::RoundToInt(Pair.Value * LootPercentage);

		if (LootAmount > 0)
		{
			LootedResources.Add(Pair.Key, LootAmount);
			Pair.Value -= LootAmount;

			UE_LOG(LogTemp, Warning, TEXT("Caravan lost %d %s to looters"),
				LootAmount, *UEnum::GetValueAsString(Pair.Key));
		}
	}

	// 0이 된 자원 제거
	for (auto It = CargoResources.CreateIterator(); It; ++It)
	{
		if (It.Value() <= 0)
		{
			It.RemoveCurrent();
		}
	}

	return LootedResources;
}

void ACaravan::InitializeCaravan(
	ATradingPost* Origin,
	ATradingPost* Destination,
	TMap<EResourceType, int32> Resources,
	int32 Guards)
{
	OriginTradingPost = Origin;
	DestinationTradingPost = Destination;
	CargoResources = Resources;
	GuardCount = Guards;

	if (Origin)
	{
		OwnerFactionID = Origin->OwnerFactionID;
	}

	UE_LOG(LogTemp, Log, TEXT("Caravan initialized from %s to %s with %d units of cargo and %d guards"),
		Origin ? *Origin->TerritoryName : TEXT("Unknown"),
		Destination ? *Destination->TerritoryName : TEXT("Unknown"),
		GetCurrentCargoAmount(),
		Guards);
}

void ACaravan::StartJourney()
{
	if (!DestinationTradingPost)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot start journey: No destination"));
		return;
	}

	CaravanState = ECaravanState::Traveling;
	TargetLocation = DestinationTradingPost->GetActorLocation();
	TravelProgress = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Caravan started journey to %s (distance: %.0f units)"),
		*DestinationTradingPost->TerritoryName,
		FVector::Dist(CurrentLocation, TargetLocation));
}

void ACaravan::ArrivedAtDestination()
{
	CaravanState = ECaravanState::Arrived;

	UE_LOG(LogTemp, Log, TEXT("Caravan arrived at %s"),
		DestinationTradingPost ? *DestinationTradingPost->TerritoryName : TEXT("Unknown"));

	// 목적지에 화물 인도
	if (DestinationTradingPost)
	{
		DestinationTradingPost->ReceiveCaravan(this);
	}

	// 호위 부대 해제
	ReleaseGuardUnit();

	// 상단 제거
	SetLifeSpan(1.0f);
}

void ACaravan::DestroyCaravan()
{
	CaravanState = ECaravanState::Destroyed;

	UE_LOG(LogTemp, Warning, TEXT("Caravan destroyed"));

	// 호위 부대 해제
	ReleaseGuardUnit();

	// 남은 화물 제거
	CargoResources.Empty();

	// 액터 제거
	SetLifeSpan(1.0f);
}

float ACaravan::GetEstimatedArrivalTime() const
{
	if (!DestinationTradingPost || MovementSpeed <= 0.0f) return -1.0f;

	float RemainingDistance = FVector::Dist(CurrentLocation, TargetLocation);
	return RemainingDistance / MovementSpeed;
}

int32 ACaravan::GetTotalCargoValue() const
{
	// TODO: 자원별 가치 계산 (현재는 단순 합산)
	return GetCurrentCargoAmount();
}
