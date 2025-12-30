// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimulatorTypes.h"
#include "Caravan.generated.h"

/**
 * 상단 상태
 */
UENUM(BlueprintType)
enum class ECaravanState : uint8
{
	Idle         UMETA(DisplayName = "Idle"),           // 대기 중
	Traveling    UMETA(DisplayName = "Traveling"),      // 이동 중
	InCombat     UMETA(DisplayName = "In Combat"),      // 전투 중
	Arrived      UMETA(DisplayName = "Arrived"),        // 도착
	Destroyed    UMETA(DisplayName = "Destroyed")       // 파괴됨
};

/**
 * 상단 액터
 * 영지 간 자원을 운반하며, 호위 병력을 동반할 수 있음
 * 적대 세력의 습격을 받을 수 있고, 전투에서 패배하면 자원 약탈당함
 */
UCLASS()
class SIMULATOR_API ACaravan : public AActor
{
	GENERATED_BODY()

public:
	ACaravan();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

	// === Caravan Info ===

	// 상단 상태
	UPROPERTY(BlueprintReadOnly, Category = "Caravan")
	ECaravanState CaravanState;

	// 출발지 교역소
	UPROPERTY(BlueprintReadOnly, Category = "Caravan")
	class ATradingPost* OriginTradingPost;

	// 목적지 교역소
	UPROPERTY(BlueprintReadOnly, Category = "Caravan")
	class ATradingPost* DestinationTradingPost;

	// 상단 소유 팩션
	UPROPERTY(BlueprintReadOnly, Category = "Caravan")
	int32 OwnerFactionID;

	// === Cargo (운반 자원) ===

	// 운반 중인 자원
	UPROPERTY(BlueprintReadOnly, Category = "Caravan|Cargo")
	TMap<EResourceType, int32> CargoResources;

	// 최대 적재량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caravan|Cargo")
	int32 MaxCargoCapacity;

	// 현재 적재량
	UFUNCTION(BlueprintCallable, Category = "Caravan|Cargo")
	int32 GetCurrentCargoAmount() const;

	// 자원 추가
	UFUNCTION(BlueprintCallable, Category = "Caravan|Cargo")
	bool AddCargo(EResourceType ResourceType, int32 Amount);

	// 자원 제거 (약탈 시)
	UFUNCTION(BlueprintCallable, Category = "Caravan|Cargo")
	bool RemoveCargo(EResourceType ResourceType, int32 Amount);

	// 모든 화물 인출 (약탈용)
	UFUNCTION(BlueprintCallable, Category = "Caravan|Cargo")
	TMap<EResourceType, int32> ExtractAllCargo();

	// === Guards (호위 병력) ===

	// 호위 부대 (MilitaryUnit 연결)
	UPROPERTY(BlueprintReadOnly, Category = "Caravan|Guards")
	class AMilitaryUnit* GuardUnit;

	// 호위 병력 수
	UPROPERTY(BlueprintReadOnly, Category = "Caravan|Guards")
	int32 GuardCount;

	// 호위 부대 배치
	UFUNCTION(BlueprintCallable, Category = "Caravan|Guards")
	void AssignGuardUnit(class AMilitaryUnit* Unit);

	// 호위 부대 해제
	UFUNCTION(BlueprintCallable, Category = "Caravan|Guards")
	void ReleaseGuardUnit();

	// === Movement ===

	// 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caravan|Movement")
	float MovementSpeed;

	// 현재 위치
	UPROPERTY(BlueprintReadOnly, Category = "Caravan|Movement")
	FVector CurrentLocation;

	// 목표 위치
	UPROPERTY(BlueprintReadOnly, Category = "Caravan|Movement")
	FVector TargetLocation;

	// 이동 진행률 (0.0 ~ 1.0)
	UPROPERTY(BlueprintReadOnly, Category = "Caravan|Movement")
	float TravelProgress;

	// 이동 업데이트
	void UpdateMovement(float DeltaTime);

	// === Combat ===

	// 전투 중인지 여부
	UPROPERTY(BlueprintReadOnly, Category = "Caravan|Combat")
	bool bIsInCombat;

	// 참가 중인 전투
	UPROPERTY(BlueprintReadOnly, Category = "Caravan|Combat")
	class ACombatEncounter* CurrentCombat;

	// 전투 진입
	UFUNCTION(BlueprintCallable, Category = "Caravan|Combat")
	void EnterCombat(class ACombatEncounter* Combat);

	// 전투 종료
	UFUNCTION(BlueprintCallable, Category = "Caravan|Combat")
	void LeaveCombat(bool bVictory);

	// 약탈당함 (전투 패배 시)
	UFUNCTION(BlueprintCallable, Category = "Caravan|Combat")
	TMap<EResourceType, int32> GetLooted(float LootPercentage = 0.5f);

	// === Initialization ===

	// 상단 초기화
	UFUNCTION(BlueprintCallable, Category = "Caravan")
	void InitializeCaravan(
		class ATradingPost* Origin,
		class ATradingPost* Destination,
		TMap<EResourceType, int32> Resources,
		int32 Guards = 0
	);

	// 여행 시작
	UFUNCTION(BlueprintCallable, Category = "Caravan")
	void StartJourney();

	// 목적지 도착
	UFUNCTION(BlueprintCallable, Category = "Caravan")
	void ArrivedAtDestination();

	// 상단 파괴 (전투 패배 또는 완전 약탈)
	UFUNCTION(BlueprintCallable, Category = "Caravan")
	void DestroyCaravan();

	// === Helper Functions ===

	// 예상 도착 시간 (초)
	UFUNCTION(BlueprintCallable, Category = "Caravan")
	float GetEstimatedArrivalTime() const;

	// 총 자산 가치 계산
	UFUNCTION(BlueprintCallable, Category = "Caravan")
	int32 GetTotalCargoValue() const;
};
