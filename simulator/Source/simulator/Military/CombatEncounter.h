// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatEncounter.generated.h"

/**
 * 전투 상태
 */
UENUM(BlueprintType)
enum class ECombatState : uint8
{
	Inactive     UMETA(DisplayName = "Inactive"),      // 전투 없음
	Engaged      UMETA(DisplayName = "Engaged"),       // 교전 중
	Retreating   UMETA(DisplayName = "Retreating"),    // 후퇴 중
	Finished     UMETA(DisplayName = "Finished")       // 전투 종료
};

/**
 * 전투 참가 정보
 */
USTRUCT(BlueprintType)
struct FCombatParticipant
{
	GENERATED_BODY()

	// 참가 부대
	UPROPERTY(BlueprintReadOnly)
	class AMilitaryUnit* Unit;

	// 전투 시작 시 병력
	UPROPERTY(BlueprintReadOnly)
	int32 InitialStrength;

	// 현재 병력
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentStrength;

	// 누적 피해량
	UPROPERTY(BlueprintReadOnly)
	int32 TotalCasualties;

	// 사기 (0.0 ~ 1.0, 0이면 도주)
	UPROPERTY(BlueprintReadOnly)
	float Morale;

	FCombatParticipant()
		: Unit(nullptr)
		, InitialStrength(0)
		, CurrentStrength(0)
		, TotalCasualties(0)
		, Morale(1.0f)
	{}

	FCombatParticipant(class AMilitaryUnit* InUnit, int32 Strength)
		: Unit(InUnit)
		, InitialStrength(Strength)
		, CurrentStrength(Strength)
		, TotalCasualties(0)
		, Morale(1.0f)
	{}
};

/**
 * 전투 발생 액터
 * 특정 위치(타일/셀)에서 여러 부대가 교전할 때 생성됨
 * 턴마다 전투 결과를 계산하고 이펙트를 표시
 */
UCLASS()
class SIMULATOR_API ACombatEncounter : public AActor
{
	GENERATED_BODY()

public:
	ACombatEncounter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// === Combat State ===

	// 전투 상태
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	ECombatState CombatState;

	// 전투 위치 (타일 중심)
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FVector CombatLocation;

	// 전투 진행 턴 수
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 TurnCount;

	// === Participants ===

	// 전투 참가 부대들
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TArray<FCombatParticipant> Participants;

	// === Combat Settings ===

	// 턴당 전투 주기 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Settings")
	float CombatTurnInterval;

	// 사기 감소율 (사상자 비율에 따라)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Settings")
	float MoraleDecayRate;

	// 후퇴 사기 임계값 (이 값 이하면 후퇴)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Settings")
	float RetreatMoraleThreshold;

	// === Visual Effects ===

	// 전투 이펙트 (안개/연기)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Effects")
	class UParticleSystemComponent* CombatEffectComponent;

	// 이펙트 크기 (참가 병력에 따라 스케일)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Effects")
	float EffectScale;

	// === Functions ===

	// 전투 시작 (여러 부대 참가)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartCombat(TArray<class AMilitaryUnit*> Units, FVector Location);

	// 부대 추가 (전투 중 합류)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool AddUnit(class AMilitaryUnit* Unit);

	// 부대 제거 (후퇴/전멸)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool RemoveUnit(class AMilitaryUnit* Unit);

	// 전투 종료
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndCombat();

	// 전투 중인지 확인
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsCombatActive() const { return CombatState == ECombatState::Engaged; }

	// 전투 참가 부대 수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	int32 GetParticipantCount() const { return Participants.Num(); }

	// 총 전투 병력 수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	int32 GetTotalCombatants() const;

	// 승자 확인 (한 쪽만 남았을 때)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	class AMilitaryUnit* GetWinner() const;

protected:
	// 턴 타이머
	float TurnTimer;

	// 턴 처리
	void ProcessCombatTurn();

	// 전투 계산 (모든 참가자 간 교전)
	void CalculateCombatDamage();

	// 특정 부대에 피해 적용
	void ApplyDamageToUnit(FCombatParticipant& Participant, int32 Damage);

	// 사기 업데이트
	void UpdateMorale();

	// 후퇴 체크 (사기가 낮은 부대 후퇴)
	void CheckRetreat();

	// 전투 종료 조건 체크
	bool CheckCombatEnd();

	// 이펙트 업데이트 (병력 수에 따라 크기 조정)
	void UpdateCombatEffect();

	// 참가자 찾기
	FCombatParticipant* FindParticipant(class AMilitaryUnit* Unit);
};
