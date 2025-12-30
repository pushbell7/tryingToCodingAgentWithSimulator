// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatManagerSubsystem.generated.h"

/**
 * 전투 시스템 관리 서브시스템
 * 맵 전체의 전투들을 관리하고, 부대 충돌 감지, 전투 시작/종료 처리
 */
UCLASS()
class SIMULATOR_API UCombatManagerSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// UTickableWorldSubsystem implementation
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	// === Combat Management ===

	// 전투 시작 (특정 위치에서 여러 부대 교전)
	UFUNCTION(BlueprintCallable, Category = "Combat Manager")
	class ACombatEncounter* StartCombat(TArray<class AMilitaryUnit*> Units, FVector Location);

	// 전투 종료
	UFUNCTION(BlueprintCallable, Category = "Combat Manager")
	void EndCombat(class ACombatEncounter* Combat);

	// 특정 위치에서 전투 중인지 확인
	UFUNCTION(BlueprintCallable, Category = "Combat Manager")
	class ACombatEncounter* GetCombatAtLocation(FVector Location, float Radius = 500.0f) const;

	// 부대가 전투 중인지 확인
	UFUNCTION(BlueprintCallable, Category = "Combat Manager")
	bool IsUnitInCombat(class AMilitaryUnit* Unit) const;

	// 부대가 참가 중인 전투 찾기
	UFUNCTION(BlueprintCallable, Category = "Combat Manager")
	class ACombatEncounter* FindCombatForUnit(class AMilitaryUnit* Unit) const;

	// 현재 진행 중인 전투 수
	UFUNCTION(BlueprintCallable, Category = "Combat Manager")
	int32 GetActiveCombatCount() const { return ActiveCombats.Num(); }

	// === Unit Collision Detection ===

	// 특정 위치에서 적대적인 부대 찾기
	UFUNCTION(BlueprintCallable, Category = "Combat Manager")
	TArray<class AMilitaryUnit*> FindHostileUnitsAtLocation(class AMilitaryUnit* Unit, FVector Location, float Radius = 500.0f) const;

	// 부대 간 충돌 체크 (자동 전투 시작)
	UFUNCTION(BlueprintCallable, Category = "Combat Manager")
	void CheckUnitCollisions();

	// === Settings ===

	// 자동 전투 시작 활성화
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Manager|Settings")
	bool bAutoStartCombat;

	// 충돌 감지 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Manager|Settings")
	float CollisionDetectionRadius;

	// 충돌 체크 주기 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Manager|Settings")
	float CollisionCheckInterval;

protected:
	// 진행 중인 전투 목록
	UPROPERTY()
	TArray<class ACombatEncounter*> ActiveCombats;

	// 등록된 모든 부대 (충돌 감지용)
	UPROPERTY()
	TArray<class AMilitaryUnit*> RegisteredUnits;

	// 충돌 체크 타이머
	float CollisionCheckTimer;

	// 전투 완료된 것들 정리
	void CleanupFinishedCombats();

	// 두 부대가 적대적인지 확인 (TODO: 팩션 시스템 연동)
	bool AreUnitsHostile(class AMilitaryUnit* UnitA, class AMilitaryUnit* UnitB) const;

public:
	// 부대 등록 (충돌 감지 대상에 추가)
	UFUNCTION(BlueprintCallable, Category = "Combat Manager")
	void RegisterUnit(class AMilitaryUnit* Unit);

	// 부대 등록 해제
	UFUNCTION(BlueprintCallable, Category = "Combat Manager")
	void UnregisterUnit(class AMilitaryUnit* Unit);

	// 모든 등록된 부대 목록
	UFUNCTION(BlueprintCallable, Category = "Combat Manager")
	TArray<class AMilitaryUnit*> GetAllUnits() const { return RegisteredUnits; }
};
