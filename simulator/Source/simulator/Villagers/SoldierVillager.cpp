// Copyright Epic Games, Inc. All Rights Reserved.

#include "SoldierVillager.h"
#include "MilitaryUnit.h"
#include "Barracks.h"

ASoldierVillager::ASoldierVillager()
{
	// 기본 전투 스탯
	AttackPower = 10.0f;
	DefensePower = 5.0f;
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	AttackRange = 200.0f;
	SightRange = 1000.0f;

	// 군사 속성
	AssignedUnit = nullptr;
	HomeBarracks = nullptr;
	FormationPosition = -1;

	// 장비
	WeaponType = TEXT("Sword");
	ArmorType = TEXT("Leather");

	// 군인은 Soldier 계급
	SocialClass = ESocialClass::Soldier;
	VillagerRole = EVillagerRole::Guard;
}

void ASoldierVillager::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
}

bool ASoldierVillager::AssignToUnit(AMilitaryUnit* Unit, int32 Position)
{
	if (!Unit)
	{
		return false;
	}

	// 이미 다른 부대에 소속되어 있으면 해제
	if (AssignedUnit && AssignedUnit != Unit)
	{
		UnassignFromUnit();
	}

	AssignedUnit = Unit;
	FormationPosition = Position;

	UE_LOG(LogTemp, Log, TEXT("%s: Assigned to unit %s at position %d"),
		*VillagerName, *Unit->GetName(), Position);

	return true;
}

void ASoldierVillager::UnassignFromUnit()
{
	if (AssignedUnit)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Unassigned from unit %s"),
			*VillagerName, *AssignedUnit->GetName());

		AssignedUnit = nullptr;
		FormationPosition = -1;
	}
}

bool ASoldierVillager::GarrisonAtBarracks(ABarracks* Barracks)
{
	if (!Barracks)
	{
		return false;
	}

	// 부대에서 해제
	UnassignFromUnit();

	HomeBarracks = Barracks;

	UE_LOG(LogTemp, Log, TEXT("%s: Garrisoned at %s"),
		*VillagerName, *Barracks->BuildingName);

	return true;
}

void ASoldierVillager::LeaveBarracks()
{
	if (HomeBarracks)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Left barracks %s"),
			*VillagerName, *HomeBarracks->BuildingName);

		HomeBarracks = nullptr;
	}
}

void ASoldierVillager::Heal(float Amount)
{
	CurrentHealth = FMath::Min(CurrentHealth + Amount, MaxHealth);

	UE_LOG(LogTemp, Log, TEXT("%s: Healed %.1f HP (%.1f/%.1f)"),
		*VillagerName, Amount, CurrentHealth, MaxHealth);
}

void ASoldierVillager::TakeCombatDamage(float Damage)
{
	// 방어력으로 피해 감소
	float ActualDamage = FMath::Max(Damage - DefensePower, 0.0f);
	CurrentHealth -= ActualDamage;

	UE_LOG(LogTemp, Warning, TEXT("%s: Took %.1f damage (%.1f/%.1f HP)"),
		*VillagerName, ActualDamage, CurrentHealth, MaxHealth);

	if (IsDead())
	{
		Die();
	}
}

bool ASoldierVillager::CanFight() const
{
	return !IsDead() && CurrentHealth > MaxHealth * 0.2f; // 20% 이상 체력 필요
}

bool ASoldierVillager::IsDead() const
{
	return CurrentHealth <= 0.0f;
}

void ASoldierVillager::Die()
{
	UE_LOG(LogTemp, Error, TEXT("%s: Died in combat"), *VillagerName);

	// 부대에서 제거
	UnassignFromUnit();

	// 병영에서 제거
	HomeBarracks = nullptr;

	CurrentState = EActorState::IDLE;

	// 액터 제거 (나중에 사망 애니메이션 등 추가 가능)
	// Destroy();
}
