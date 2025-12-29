// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MilitaryUnit.generated.h"

/**
 * 대형 타입
 */
UENUM(BlueprintType)
enum class EFormationType : uint8
{
	Line       UMETA(DisplayName = "Line"),        // 횡대 - 넓은 전선
	Column     UMETA(DisplayName = "Column"),      // 종대 - 이동에 유리
	Box        UMETA(DisplayName = "Box"),         // 방진 - 방어에 유리
	Wedge      UMETA(DisplayName = "Wedge"),       // 쐐기 - 돌파에 유리
	Scatter    UMETA(DisplayName = "Scatter")      // 산개 - 원거리 공격
};

/**
 * 부대 액터 - 여러 군인을 하나의 부대로 관리
 * 단체 이동, 대형 유지, 부대 단위 전투
 */
UCLASS()
class SIMULATOR_API AMilitaryUnit : public AActor
{
	GENERATED_BODY()

public:
	AMilitaryUnit();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// === Unit Composition ===

	// 부대 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	FString UnitName;

	// 부대원 목록
	UPROPERTY(BlueprintReadOnly, Category = "Unit")
	TArray<class ASoldierVillager*> Soldiers;

	// 최대 부대 인원
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	int32 MaxUnitSize;

	// 현재 부대 인원
	UFUNCTION(BlueprintCallable, Category = "Unit")
	int32 GetUnitSize() const { return Soldiers.Num(); }

	// 부대 지휘관 (첫번째 병사 또는 별도 지정)
	UPROPERTY(BlueprintReadOnly, Category = "Unit")
	class ASoldierVillager* Commander;

	// === Formation ===

	// 현재 대형
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	EFormationType CurrentFormation;

	// 대형 간격 (병사 간 거리)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	float FormationSpacing;

	// 부대 중심 위치
	UPROPERTY(BlueprintReadOnly, Category = "Formation")
	FVector FormationCenter;

	// 부대 방향
	UPROPERTY(BlueprintReadOnly, Category = "Formation")
	FRotator FormationRotation;

	// === Movement ===

	// 목표 위치
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector TargetLocation;

	// 이동 중인지 여부
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsMoving;

	// 이동 속도 (가장 느린 병사 기준)
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float MovementSpeed;

	// === Functions ===

	// 병사 추가
	UFUNCTION(BlueprintCallable, Category = "Unit")
	bool AddSoldier(class ASoldierVillager* Soldier, int32 Position = -1);

	// 병사 제거
	UFUNCTION(BlueprintCallable, Category = "Unit")
	bool RemoveSoldier(class ASoldierVillager* Soldier);

	// 부대 해산 (모든 병사 제거)
	UFUNCTION(BlueprintCallable, Category = "Unit")
	void DisbandUnit();

	// 대형 변경
	UFUNCTION(BlueprintCallable, Category = "Formation")
	void SetFormation(EFormationType NewFormation);

	// 특정 위치로 이동 명령
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveToLocation(FVector Location);

	// 이동 정지
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopMovement();

	// 대형 위치 계산 (병사의 대형 내 위치)
	UFUNCTION(BlueprintCallable, Category = "Formation")
	FVector GetFormationPosition(int32 SoldierIndex) const;

	// 부대 중심 업데이트
	UFUNCTION(BlueprintCallable, Category = "Formation")
	void UpdateFormationCenter();

	// 전투 가능 병력 수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	int32 GetCombatReadyCount() const;

	// 평균 체력
	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetAverageHealth() const;

protected:
	// 대형 유지 (병사들을 대형 위치로 이동)
	void MaintainFormation(float DeltaTime);

	// 이동 업데이트
	void UpdateMovement(float DeltaTime);

	// Line 대형 위치 계산
	FVector CalculateLineFormationPosition(int32 Index) const;

	// Column 대형 위치 계산
	FVector CalculateColumnFormationPosition(int32 Index) const;

	// Box 대형 위치 계산
	FVector CalculateBoxFormationPosition(int32 Index) const;

	// Wedge 대형 위치 계산
	FVector CalculateWedgeFormationPosition(int32 Index) const;

	// Scatter 대형 위치 계산
	FVector CalculateScatterFormationPosition(int32 Index) const;
};
