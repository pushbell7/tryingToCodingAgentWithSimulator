// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SimulatorTypes.h"
#include "ResourceManagerSubsystem.generated.h"

/**
 * 영지 전체의 자원을 관리하는 WorldSubsystem
 * 모든 창고의 자원을 합산하여 총 자원량 추적
 * 건설 비용 검증 및 자원 차감
 */
UCLASS()
class SIMULATOR_API UResourceManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// === 자원 조회 ===

	// 특정 자원의 총량 조회 (모든 창고 합산)
	UFUNCTION(BlueprintCallable, Category = "Resource Manager")
	int32 GetTotalResource(EResourceType ResourceType) const;

	// 특정 자원이 충분한지 확인
	UFUNCTION(BlueprintCallable, Category = "Resource Manager")
	bool HasEnoughResource(EResourceType ResourceType, int32 RequiredAmount) const;

	// 여러 자원이 모두 충분한지 확인
	UFUNCTION(BlueprintCallable, Category = "Resource Manager")
	bool HasEnoughResources(const TArray<FResourceStack>& RequiredResources) const;

	// 모든 자원 현황 조회 (타입별 총량)
	UFUNCTION(BlueprintCallable, Category = "Resource Manager")
	TMap<EResourceType, int32> GetAllResourceTotals() const;

	// === 자원 차감/추가 ===

	// 특정 자원 차감 (모든 창고에서 분산 차감)
	UFUNCTION(BlueprintCallable, Category = "Resource Manager")
	bool DeductResource(EResourceType ResourceType, int32 Amount);

	// 여러 자원 한번에 차감
	UFUNCTION(BlueprintCallable, Category = "Resource Manager")
	bool DeductResources(const TArray<FResourceStack>& Resources);

	// 자원 반환 (가장 가까운 창고에 추가)
	UFUNCTION(BlueprintCallable, Category = "Resource Manager")
	bool RefundResources(const TArray<FResourceStack>& Resources, FVector Location);

	// === 건설 비용 검증 ===

	// 건설 비용이 충분한지 확인
	UFUNCTION(BlueprintCallable, Category = "Resource Manager|Construction")
	bool CanAffordConstruction(const FConstructionCost& Cost) const;

	// 건설 비용 차감
	UFUNCTION(BlueprintCallable, Category = "Resource Manager|Construction")
	bool PayConstructionCost(const FConstructionCost& Cost);

	// 건설 취소 시 자원 반환
	UFUNCTION(BlueprintCallable, Category = "Resource Manager|Construction")
	bool RefundConstructionCost(const FConstructionCost& Cost, FVector Location);

	// === 디버그/통계 ===

	// 자원 현황 로그 출력
	UFUNCTION(BlueprintCallable, Category = "Resource Manager|Debug")
	void LogResourceStatus() const;

	// 특정 자원의 상세 정보 (어느 창고에 얼마나 있는지)
	UFUNCTION(BlueprintCallable, Category = "Resource Manager|Debug")
	void LogResourceDetails(EResourceType ResourceType) const;

protected:
	// 주기적으로 자원 캐시 갱신
	void RefreshResourceCache();

	// 타이머 핸들
	FTimerHandle RefreshTimerHandle;

	// 갱신 주기 (초)
	float RefreshInterval;

	// 캐시된 자원 총량 (성능 최적화용)
	UPROPERTY()
	TMap<EResourceType, int32> CachedResourceTotals;

	// 마지막 갱신 시간
	float LastRefreshTime;
};
