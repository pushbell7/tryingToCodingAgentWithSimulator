// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimulatorTypes.h"
#include "Territory.generated.h"

/**
 * 영지 액터
 * 특정 지역의 자원, 건물, 주민, 생산을 통합 관리
 * 자원은 영지 단위로 중앙 집중 관리
 */
UCLASS()
class SIMULATOR_API ATerritory : public AActor
{
	GENERATED_BODY()

public:
	ATerritory();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// === Territory Info ===

	// 영지 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	FString TerritoryName;

	// 소유 팩션 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	int32 OwnerFactionID;

	// 영지 중심 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	FVector TerritoryCenter;

	// 영지 범위 (반경)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	float TerritoryRadius;

	// === Resource Management ===

	// 영지 자원 저장소 (중앙 집중)
	UPROPERTY(BlueprintReadOnly, Category = "Territory|Resources")
	TMap<EResourceType, int32> TerritoryResources;

	// 최대 저장 용량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory|Resources")
	int32 MaxStorageCapacity;

	// 현재 저장량
	UFUNCTION(BlueprintCallable, Category = "Territory|Resources")
	int32 GetTotalResourceAmount() const;

	// 자원 추가
	UFUNCTION(BlueprintCallable, Category = "Territory|Resources")
	bool AddResource(EResourceType ResourceType, int32 Amount);

	// 자원 제거
	UFUNCTION(BlueprintCallable, Category = "Territory|Resources")
	bool RemoveResource(EResourceType ResourceType, int32 Amount);

	// 특정 자원 보유량
	UFUNCTION(BlueprintCallable, Category = "Territory|Resources")
	int32 GetResourceAmount(EResourceType ResourceType) const;

	// 자원 여유 확인
	UFUNCTION(BlueprintCallable, Category = "Territory|Resources")
	bool HasResource(EResourceType ResourceType, int32 Amount) const;

	// === Buildings ===

	// 영지 내 건물들
	UPROPERTY(BlueprintReadOnly, Category = "Territory|Buildings")
	TArray<class ABaseBuilding*> Buildings;

	// 교역소 (대외 무역 창구)
	UPROPERTY(BlueprintReadOnly, Category = "Territory|Buildings")
	class ATradingPost* TradingPost;

	// 건물 등록
	UFUNCTION(BlueprintCallable, Category = "Territory|Buildings")
	void RegisterBuilding(class ABaseBuilding* Building);

	// 건물 등록 해제
	UFUNCTION(BlueprintCallable, Category = "Territory|Buildings")
	void UnregisterBuilding(class ABaseBuilding* Building);

	// 교역소 설정
	UFUNCTION(BlueprintCallable, Category = "Territory|Buildings")
	void SetTradingPost(class ATradingPost* Post);

	// === Population ===

	// 영지 주민들
	UPROPERTY(BlueprintReadOnly, Category = "Territory|Population")
	TArray<class ABaseVillager*> Villagers;

	// 인구 수
	UFUNCTION(BlueprintCallable, Category = "Territory|Population")
	int32 GetPopulation() const { return Villagers.Num(); }

	// 주민 등록
	UFUNCTION(BlueprintCallable, Category = "Territory|Population")
	void RegisterVillager(class ABaseVillager* Villager);

	// 주민 등록 해제
	UFUNCTION(BlueprintCallable, Category = "Territory|Population")
	void UnregisterVillager(class ABaseVillager* Villager);

	// === Production & Consumption ===

	// 턴당 자원 생산량 (건물/주민 활동 결과)
	UPROPERTY(BlueprintReadOnly, Category = "Territory|Economy")
	TMap<EResourceType, int32> ProductionPerTurn;

	// 턴당 자원 소비량 (주민 유지비)
	UPROPERTY(BlueprintReadOnly, Category = "Territory|Economy")
	TMap<EResourceType, int32> ConsumptionPerTurn;

	// 생산/소비 계산
	UFUNCTION(BlueprintCallable, Category = "Territory|Economy")
	void CalculateProduction();

	UFUNCTION(BlueprintCallable, Category = "Territory|Economy")
	void CalculateConsumption();

	// 턴 처리 (생산-소비 적용)
	UFUNCTION(BlueprintCallable, Category = "Territory|Economy")
	void ProcessTurn();

	// === Trade ===

	// 다른 영지로 자원 수출 (교역소 통해)
	UFUNCTION(BlueprintCallable, Category = "Territory|Trade")
	class ACaravan* ExportResources(
		ATerritory* Destination,
		TMap<EResourceType, int32> Resources,
		int32 GuardCount = 0
	);

	// 수입 자원 수령
	UFUNCTION(BlueprintCallable, Category = "Territory|Trade")
	void ImportResources(TMap<EResourceType, int32> Resources);

	// === Helper Functions ===

	// 액터가 영지 내에 있는지 확인
	UFUNCTION(BlueprintCallable, Category = "Territory")
	bool IsActorInTerritory(AActor* Actor) const;

	// 위치가 영지 내에 있는지 확인
	UFUNCTION(BlueprintCallable, Category = "Territory")
	bool IsLocationInTerritory(FVector Location) const;

	// 다른 영지까지의 거리
	UFUNCTION(BlueprintCallable, Category = "Territory")
	float GetDistanceToTerritory(ATerritory* Other) const;
};
