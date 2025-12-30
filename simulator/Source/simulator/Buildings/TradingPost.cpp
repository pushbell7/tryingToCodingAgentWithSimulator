// Copyright Epic Games, Inc. All Rights Reserved.

#include "TradingPost.h"
#include "Caravan.h"

ATradingPost::ATradingPost()
{
	PrimaryActorTick.bCanEverTick = true;

	// 기본 건물 정보
	BuildingType = EBuildingType::Market;
	BuildingName = TEXT("Trading Post");

	// 건설 비용
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Wood, 150));
	ConstructionCost.RequiredResources.Add(FResourceStack(EResourceType::Stone, 100));
	ConstructionCost.RequiredWorkAmount = 200.0f;
	ConstructionCost.MaxWorkers = 5;

	// 영지 정보
	TerritoryName = TEXT("Unassigned");
	OwnerFactionID = 0;

	// 창고 설정
	MaxStorageCapacity = 1000;

	// 자동 교역 설정
	bAutoTrade = false;
	AutoTradeInterval = 60.0f; // 60초마다
	AutoTradeTimer = 0.0f;
}

void ATradingPost::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("TradingPost %s created in territory %s"),
		*BuildingName, *TerritoryName);
}

void ATradingPost::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bAutoTrade && bIsOperational)
	{
		AutoTradeTimer += DeltaTime;

		if (AutoTradeTimer >= AutoTradeInterval)
		{
			AutoTradeTimer = 0.0f;
			ProcessAutoTrade();
		}
	}
}

void ATradingPost::ConnectToTradingPost(ATradingPost* Other)
{
	if (!Other || Other == this) return;

	// 양방향 연결
	if (!ConnectedTradingPosts.Contains(Other))
	{
		ConnectedTradingPosts.Add(Other);
		UE_LOG(LogTemp, Log, TEXT("TradingPost %s connected to %s"),
			*TerritoryName, *Other->TerritoryName);
	}

	if (!Other->ConnectedTradingPosts.Contains(this))
	{
		Other->ConnectedTradingPosts.Add(this);
	}
}

void ATradingPost::DisconnectFromTradingPost(ATradingPost* Other)
{
	if (!Other) return;

	ConnectedTradingPosts.Remove(Other);
	Other->ConnectedTradingPosts.Remove(this);

	UE_LOG(LogTemp, Log, TEXT("TradingPost %s disconnected from %s"),
		*TerritoryName, *Other->TerritoryName);
}

bool ATradingPost::IsConnectedTo(ATradingPost* Other) const
{
	return ConnectedTradingPosts.Contains(Other);
}

int32 ATradingPost::GetCurrentStorageAmount() const
{
	int32 Total = 0;
	for (const auto& Pair : StoredResources)
	{
		Total += Pair.Value;
	}
	return Total;
}

bool ATradingPost::HasStorageSpace(int32 Amount) const
{
	return (GetCurrentStorageAmount() + Amount) <= MaxStorageCapacity;
}

bool ATradingPost::StoreResource(EResourceType ResourceType, int32 Amount)
{
	if (Amount <= 0) return false;

	if (!HasStorageSpace(Amount))
	{
		UE_LOG(LogTemp, Warning, TEXT("TradingPost %s: Not enough storage space"),
			*TerritoryName);
		return false;
	}

	if (!StoredResources.Contains(ResourceType))
	{
		StoredResources.Add(ResourceType, 0);
	}

	StoredResources[ResourceType] += Amount;

	UE_LOG(LogTemp, Log, TEXT("TradingPost %s stored %d %s (total: %d)"),
		*TerritoryName, Amount,
		*UEnum::GetValueAsString(ResourceType),
		StoredResources[ResourceType]);

	return true;
}

bool ATradingPost::WithdrawResource(EResourceType ResourceType, int32 Amount)
{
	if (Amount <= 0) return false;

	if (!StoredResources.Contains(ResourceType) || StoredResources[ResourceType] < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("TradingPost %s: Not enough %s to withdraw"),
			*TerritoryName, *UEnum::GetValueAsString(ResourceType));
		return false;
	}

	StoredResources[ResourceType] -= Amount;

	UE_LOG(LogTemp, Log, TEXT("TradingPost %s withdrew %d %s (remaining: %d)"),
		*TerritoryName, Amount,
		*UEnum::GetValueAsString(ResourceType),
		StoredResources[ResourceType]);

	return true;
}

int32 ATradingPost::GetResourceAmount(EResourceType ResourceType) const
{
	if (StoredResources.Contains(ResourceType))
	{
		return StoredResources[ResourceType];
	}
	return 0;
}

ACaravan* ATradingPost::SendCaravan(
	ATradingPost* Destination,
	TMap<EResourceType, int32> Resources,
	int32 GuardCount)
{
	if (!Destination)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot send caravan: Invalid destination"));
		return nullptr;
	}

	if (!CanTradeWith(Destination))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot trade with %s"), *Destination->TerritoryName);
		return nullptr;
	}

	// 자원 인출 확인
	for (const auto& Pair : Resources)
	{
		if (GetResourceAmount(Pair.Key) < Pair.Value)
		{
			UE_LOG(LogTemp, Warning, TEXT("Not enough %s to send"),
				*UEnum::GetValueAsString(Pair.Key));
			return nullptr;
		}
	}

	// 자원 인출
	for (const auto& Pair : Resources)
	{
		WithdrawResource(Pair.Key, Pair.Value);
	}

	// 상단 생성
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACaravan* NewCaravan = World->SpawnActor<ACaravan>(
		ACaravan::StaticClass(),
		GetActorLocation(),
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (NewCaravan)
	{
		NewCaravan->InitializeCaravan(this, Destination, Resources, GuardCount);
		RegisterCaravan(NewCaravan);

		UE_LOG(LogTemp, Log, TEXT("Caravan sent from %s to %s with %d guards"),
			*TerritoryName, *Destination->TerritoryName, GuardCount);
	}

	return NewCaravan;
}

void ATradingPost::ReceiveCaravan(ACaravan* Caravan)
{
	if (!Caravan) return;

	// 상단의 자원을 창고에 저장
	for (const auto& Pair : Caravan->CargoResources)
	{
		StoreResource(Pair.Key, Pair.Value);
	}

	UE_LOG(LogTemp, Log, TEXT("TradingPost %s received caravan from %s"),
		*TerritoryName,
		Caravan->OriginTradingPost ? *Caravan->OriginTradingPost->TerritoryName : TEXT("Unknown"));

	// 상단 제거는 Caravan이 자체적으로 처리
}

void ATradingPost::RegisterCaravan(ACaravan* Caravan)
{
	if (Caravan && !ActiveCaravans.Contains(Caravan))
	{
		ActiveCaravans.Add(Caravan);
	}
}

void ATradingPost::UnregisterCaravan(ACaravan* Caravan)
{
	ActiveCaravans.Remove(Caravan);
}

void ATradingPost::ProcessAutoTrade()
{
	// TODO: 자동 교역 로직
	// 1. 연결된 교역소 중 자원 부족한 곳 찾기
	// 2. 여유 자원 계산
	// 3. 상단 파견

	UE_LOG(LogTemp, Log, TEXT("TradingPost %s processing auto trade"), *TerritoryName);
}

bool ATradingPost::CanTradeWith(ATradingPost* Other) const
{
	if (!Other || Other == this) return false;

	// 연결 확인
	if (!IsConnectedTo(Other)) return false;

	// TODO: 적대 관계 확인 (팩션 시스템 연동)
	// 현재는 다른 팩션과도 거래 가능

	return true;
}

float ATradingPost::GetTradingPostDistance(ATradingPost* Other) const
{
	if (!Other) return -1.0f;

	return FVector::Dist(GetActorLocation(), Other->GetActorLocation());
}
