// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseVillager.h"
#include "SoldierVillager.generated.h"

/**
 * 군인 주민 - 전투와 방어를 담당
 * 병영에 주둔하고 부대에 소속되어 활동
 */
UCLASS()
class SIMULATOR_API ASoldierVillager : public ABaseVillager
{
	GENERATED_BODY()

public:
	ASoldierVillager();

protected:
	virtual void BeginPlay() override;

public:
	// === Combat Stats ===

	// 공격력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackPower;

	// 방어력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DefensePower;

	// 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MaxHealth;

	// 현재 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float CurrentHealth;

	// 공격 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRange;

	// 시야 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float SightRange;

	// === Military Assignment ===

	// 소속 부대
	UPROPERTY(BlueprintReadOnly, Category = "Military")
	class AMilitaryUnit* AssignedUnit;

	// 주둔 병영
	UPROPERTY(BlueprintReadOnly, Category = "Military")
	class ABarracks* HomeBarracks;

	// 부대 내 위치 (대형에서의 위치)
	UPROPERTY(BlueprintReadOnly, Category = "Military")
	int32 FormationPosition;

	// === Equipment ===

	// 무기 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FString WeaponType;

	// 방어구 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FString ArmorType;

	// === Functions ===

	// 부대에 배치
	UFUNCTION(BlueprintCallable, Category = "Military")
	bool AssignToUnit(class AMilitaryUnit* Unit, int32 Position);

	// 부대에서 해제
	UFUNCTION(BlueprintCallable, Category = "Military")
	void UnassignFromUnit();

	// 병영에 주둔
	UFUNCTION(BlueprintCallable, Category = "Military")
	bool GarrisonAtBarracks(class ABarracks* Barracks);

	// 병영에서 출동
	UFUNCTION(BlueprintCallable, Category = "Military")
	void LeaveBarracks();

	// 체력 회복
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Heal(float Amount);

	// 피해 입음 (커스텀 함수)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TakeCombatDamage(float Damage);

	// 전투 가능 여부
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanFight() const;

	// 사망 여부
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsDead() const;

protected:
	// 사망 처리
	virtual void Die();
};
