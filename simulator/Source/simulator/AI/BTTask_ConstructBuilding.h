// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ConstructBuilding.generated.h"

/**
 * 건설 현장에서 건물 건설 작업을 수행하는 태스크
 */
UCLASS()
class SIMULATOR_API UBTTask_ConstructBuilding : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ConstructBuilding();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	// 건설 현장 찾기
	class AConstructionSite* FindNearestConstructionSite(class ABaseVillager* Villager);

	// 작업 수행
	bool PerformConstructionWork(class AConstructionSite* Site, class ABaseVillager* Villager, float DeltaTime);

protected:
	// 건설 현장 검색 최대 거리
	UPROPERTY(EditAnywhere, Category = "Construction")
	float MaxSearchDistance;

	// 작업 반경 (건설 현장에서 이 거리 내에 있어야 작업 가능)
	UPROPERTY(EditAnywhere, Category = "Construction")
	float WorkRadius;

	// 초당 작업량 (기본값)
	UPROPERTY(EditAnywhere, Category = "Construction")
	float WorkRatePerSecond;

	// 최대 작업 시간 (초)
	UPROPERTY(EditAnywhere, Category = "Construction")
	float MaxWorkDuration;

	// 블랙보드 키 - 건설 현장
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName ConstructionSiteKey;

private:
	// 현재 작업한 시간 추적
	float WorkedTime;

	// 현재 작업 중인 건설 현장
	class AConstructionSite* CurrentSite;
};
