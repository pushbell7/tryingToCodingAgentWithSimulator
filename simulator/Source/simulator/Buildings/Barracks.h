// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseBuilding.h"
#include "Barracks.generated.h"

/**
 * 병영 - 군인들의 주둔지
 * 병력 수용, 훈련, 부대 편성 등을 담당
 */
UCLASS()
class SIMULATOR_API ABarracks : public ABaseBuilding
{
	GENERATED_BODY()

public:
	ABarracks();

protected:
	virtual void BeginPlay() override;

public:
	// === Garrison Management ===

	// 주둔 중인 병력
	UPROPERTY(BlueprintReadOnly, Category = "Barracks")
	TArray<class ASoldierVillager*> GarrisonedSoldiers;

	// 최대 주둔 인원
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barracks")
	int32 MaxGarrison;

	// 현재 주둔 인원
	UFUNCTION(BlueprintCallable, Category = "Barracks")
	int32 GetCurrentGarrison() const { return GarrisonedSoldiers.Num(); }

	// 주둔 가능 인원
	UFUNCTION(BlueprintCallable, Category = "Barracks")
	int32 GetAvailableGarrisonSlots() const { return MaxGarrison - GarrisonedSoldiers.Num(); }

	// 주둔 공간이 있는지 확인
	UFUNCTION(BlueprintCallable, Category = "Barracks")
	bool HasAvailableGarrisonSlots() const { return GetAvailableGarrisonSlots() > 0; }

	// === Soldier Management ===

	// 병력 주둔
	UFUNCTION(BlueprintCallable, Category = "Barracks")
	bool GarrisonSoldier(class ASoldierVillager* Soldier);

	// 병력 출동
	UFUNCTION(BlueprintCallable, Category = "Barracks")
	bool ReleaseSoldier(class ASoldierVillager* Soldier);

	// 모든 병력 출동
	UFUNCTION(BlueprintCallable, Category = "Barracks")
	TArray<class ASoldierVillager*> ReleaseAllSoldiers();

	// 특정 수만큼 병력 출동
	UFUNCTION(BlueprintCallable, Category = "Barracks")
	TArray<class ASoldierVillager*> ReleaseSoldiers(int32 Count);

	// === Recruitment (추후 구현) ===

	// 병력 모집 가능 여부
	UFUNCTION(BlueprintCallable, Category = "Barracks|Recruitment")
	bool CanRecruitSoldier() const;

	// 병력 모집
	UFUNCTION(BlueprintCallable, Category = "Barracks|Recruitment")
	class ASoldierVillager* RecruitSoldier();

	// === Unit Formation ===

	// 부대 편성 (주둔 병력으로 새 부대 생성)
	UFUNCTION(BlueprintCallable, Category = "Barracks|Units")
	class AMilitaryUnit* FormUnit(int32 SoldierCount);

	// === Healing ===

	// 주둔 중인 병력 치료 (매 틱마다 소량 회복)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barracks|Healing")
	float HealingRatePerSecond;

	// 체력 회복 활성화
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barracks|Healing")
	bool bEnableHealing;

protected:
	virtual void Tick(float DeltaTime) override;

	// 주둔 병력 치료
	void HealGarrisonedSoldiers(float DeltaTime);
};
