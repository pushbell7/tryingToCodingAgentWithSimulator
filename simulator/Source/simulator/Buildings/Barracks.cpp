// Copyright Epic Games, Inc. All Rights Reserved.

#include "Barracks.h"
#include "SoldierVillager.h"
#include "MilitaryUnit.h"

ABarracks::ABarracks()
{
	BuildingType = EBuildingType::TownHall; // 임시로 TownHall 사용 (나중에 Barracks 추가 필요)
	BuildingName = TEXT("Barracks");

	MaxGarrison = 20;
	MaxWorkers = 5;

	HealingRatePerSecond = 5.0f;
	bEnableHealing = true;

	PrimaryActorTick.bCanEverTick = true;

	// 건설 비용 설정
	ConstructionCost.RequiredResources = {
		FResourceStack(EResourceType::Wood, 100),
		FResourceStack(EResourceType::Stone, 50)
	};
	ConstructionCost.RequiredWorkAmount = 200.0f;
	ConstructionCost.MaxWorkers = 5;
}

void ABarracks::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("Barracks %s initialized - Max Garrison: %d"),
		*BuildingName, MaxGarrison);
}

void ABarracks::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 주둔 병력 치료
	if (bEnableHealing && bIsOperational)
	{
		HealGarrisonedSoldiers(DeltaTime);
	}
}

bool ABarracks::GarrisonSoldier(ASoldierVillager* Soldier)
{
	if (!Soldier)
	{
		return false;
	}

	// 공간 확인
	if (!HasAvailableGarrisonSlots())
	{
		UE_LOG(LogTemp, Warning, TEXT("Barracks %s: No garrison slots available (%d/%d)"),
			*BuildingName, GetCurrentGarrison(), MaxGarrison);
		return false;
	}

	// 이미 주둔 중인지 확인
	if (GarrisonedSoldiers.Contains(Soldier))
	{
		UE_LOG(LogTemp, Warning, TEXT("Barracks %s: %s already garrisoned"),
			*BuildingName, *Soldier->VillagerName);
		return false;
	}

	// 주둔 처리
	GarrisonedSoldiers.Add(Soldier);
	Soldier->GarrisonAtBarracks(this);

	UE_LOG(LogTemp, Log, TEXT("Barracks %s: Garrisoned %s (%d/%d)"),
		*BuildingName, *Soldier->VillagerName, GetCurrentGarrison(), MaxGarrison);

	return true;
}

bool ABarracks::ReleaseSoldier(ASoldierVillager* Soldier)
{
	if (!Soldier)
	{
		return false;
	}

	if (!GarrisonedSoldiers.Contains(Soldier))
	{
		UE_LOG(LogTemp, Warning, TEXT("Barracks %s: %s not garrisoned here"),
			*BuildingName, *Soldier->VillagerName);
		return false;
	}

	GarrisonedSoldiers.Remove(Soldier);
	Soldier->LeaveBarracks();

	UE_LOG(LogTemp, Log, TEXT("Barracks %s: Released %s (%d/%d)"),
		*BuildingName, *Soldier->VillagerName, GetCurrentGarrison(), MaxGarrison);

	return true;
}

TArray<ASoldierVillager*> ABarracks::ReleaseAllSoldiers()
{
	TArray<ASoldierVillager*> ReleasedSoldiers = GarrisonedSoldiers;

	for (ASoldierVillager* Soldier : ReleasedSoldiers)
	{
		if (Soldier)
		{
			Soldier->LeaveBarracks();
		}
	}

	GarrisonedSoldiers.Empty();

	UE_LOG(LogTemp, Warning, TEXT("Barracks %s: Released all %d soldiers"),
		*BuildingName, ReleasedSoldiers.Num());

	return ReleasedSoldiers;
}

TArray<ASoldierVillager*> ABarracks::ReleaseSoldiers(int32 Count)
{
	TArray<ASoldierVillager*> ReleasedSoldiers;

	int32 ReleaseCount = FMath::Min(Count, GarrisonedSoldiers.Num());

	for (int32 i = 0; i < ReleaseCount; i++)
	{
		ASoldierVillager* Soldier = GarrisonedSoldiers[0];
		if (Soldier)
		{
			ReleasedSoldiers.Add(Soldier);
			ReleaseSoldier(Soldier);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Barracks %s: Released %d soldiers (%d remaining)"),
		*BuildingName, ReleasedSoldiers.Num(), GetCurrentGarrison());

	return ReleasedSoldiers;
}

bool ABarracks::CanRecruitSoldier() const
{
	// 공간이 있고, 운영 중이어야 함
	return HasAvailableGarrisonSlots() && bIsOperational;
}

ASoldierVillager* ABarracks::RecruitSoldier()
{
	if (!CanRecruitSoldier())
	{
		UE_LOG(LogTemp, Warning, TEXT("Barracks %s: Cannot recruit soldier"), *BuildingName);
		return nullptr;
	}

	// 실제 병력 생성 (추후 구현)
	// 자원 소모, 시간 소요 등 구현 필요
	UE_LOG(LogTemp, Log, TEXT("Barracks %s: Recruiting soldier (NOT IMPLEMENTED)"), *BuildingName);

	return nullptr;
}

AMilitaryUnit* ABarracks::FormUnit(int32 SoldierCount)
{
	if (SoldierCount <= 0 || GarrisonedSoldiers.Num() < SoldierCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("Barracks %s: Not enough soldiers to form unit (need %d, have %d)"),
			*BuildingName, SoldierCount, GarrisonedSoldiers.Num());
		return nullptr;
	}

	// 부대 편성할 병력 출동
	TArray<ASoldierVillager*> UnitSoldiers = ReleaseSoldiers(SoldierCount);

	if (UnitSoldiers.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Barracks %s: Failed to release soldiers for unit"), *BuildingName);
		return nullptr;
	}

	// MilitaryUnit 생성
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMilitaryUnit* NewUnit = GetWorld()->SpawnActor<AMilitaryUnit>(
		AMilitaryUnit::StaticClass(),
		GetBuildingLocation(),
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (NewUnit)
	{
		// 병력을 부대에 배치
		for (int32 i = 0; i < UnitSoldiers.Num(); i++)
		{
			if (UnitSoldiers[i])
			{
				NewUnit->AddSoldier(UnitSoldiers[i], i);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("Barracks %s: Formed unit %s with %d soldiers"),
			*BuildingName, *NewUnit->GetName(), UnitSoldiers.Num());

		return NewUnit;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Barracks %s: Failed to spawn military unit"), *BuildingName);

		// 실패 시 병력을 다시 주둔
		for (ASoldierVillager* Soldier : UnitSoldiers)
		{
			GarrisonSoldier(Soldier);
		}

		return nullptr;
	}
}

void ABarracks::HealGarrisonedSoldiers(float DeltaTime)
{
	for (ASoldierVillager* Soldier : GarrisonedSoldiers)
	{
		if (Soldier && !Soldier->IsDead() && Soldier->CurrentHealth < Soldier->MaxHealth)
		{
			float HealAmount = HealingRatePerSecond * DeltaTime;
			Soldier->Heal(HealAmount);
		}
	}
}
