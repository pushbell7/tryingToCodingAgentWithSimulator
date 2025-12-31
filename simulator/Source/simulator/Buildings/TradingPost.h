// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseBuilding.h"
#include "SimulatorTypes.h"
#include "TradingPost.generated.h"

/**
 * 교역소 - 영지 간 무역의 거점
 * 상단 생성, 자원 교환, 교역 관리
 */
UCLASS()
class SIMULATOR_API ATradingPost : public ABaseBuilding
{
	GENERATED_BODY()

public:
	ATradingPost();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// === Territory ===
	// Note: OwnerTerritory is inherited from BaseBuilding

	// 소속 영지 (이름)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading Post")
	FString TerritoryName;

	// 영지 소유자 (팩션/플레이어)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading Post")
	int32 OwnerFactionID;

	// 영지 설정
	UFUNCTION(BlueprintCallable, Category = "Trading Post")
	void SetOwnerTerritory(class ATerritory* Territory);

	// === Trade Routes ===

	// 연결된 다른 교역소들 (교역 경로)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading Post|Routes")
	TArray<ATradingPost*> ConnectedTradingPosts;

	// 교역소 연결 (양방향)
	UFUNCTION(BlueprintCallable, Category = "Trading Post|Routes")
	void ConnectToTradingPost(ATradingPost* Other);

	// 교역소 연결 해제
	UFUNCTION(BlueprintCallable, Category = "Trading Post|Routes")
	void DisconnectFromTradingPost(ATradingPost* Other);

	// 특정 교역소와 연결되어 있는지 확인
	UFUNCTION(BlueprintCallable, Category = "Trading Post|Routes")
	bool IsConnectedTo(ATradingPost* Other) const;

	// === Resource Storage ===

	// 교역소 창고 (임시 저장)
	UPROPERTY(BlueprintReadOnly, Category = "Trading Post|Storage")
	TMap<EResourceType, int32> StoredResources;

	// 창고 최대 용량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading Post|Storage")
	int32 MaxStorageCapacity;

	// 현재 저장량
	UFUNCTION(BlueprintCallable, Category = "Trading Post|Storage")
	int32 GetCurrentStorageAmount() const;

	// 저장 공간 확인
	UFUNCTION(BlueprintCallable, Category = "Trading Post|Storage")
	bool HasStorageSpace(int32 Amount) const;

	// 자원 저장
	UFUNCTION(BlueprintCallable, Category = "Trading Post|Storage")
	bool StoreResource(EResourceType ResourceType, int32 Amount);

	// 자원 인출
	UFUNCTION(BlueprintCallable, Category = "Trading Post|Storage")
	bool WithdrawResource(EResourceType ResourceType, int32 Amount);

	// 특정 자원 보유량
	UFUNCTION(BlueprintCallable, Category = "Trading Post|Storage")
	int32 GetResourceAmount(EResourceType ResourceType) const;

	// === Caravan Management ===

	// 이 교역소에서 출발한 활성 상단들
	UPROPERTY(BlueprintReadOnly, Category = "Trading Post|Caravan")
	TArray<class ACaravan*> ActiveCaravans;

	// 상단 생성 및 파견
	UFUNCTION(BlueprintCallable, Category = "Trading Post|Caravan")
	class ACaravan* SendCaravan(
		ATradingPost* Destination,
		TMap<EResourceType, int32> Resources,
		int32 GuardCount = 0
	);

	// 상단 도착 처리
	UFUNCTION(BlueprintCallable, Category = "Trading Post|Caravan")
	void ReceiveCaravan(class ACaravan* Caravan);

	// 상단 등록
	UFUNCTION(BlueprintCallable, Category = "Trading Post|Caravan")
	void RegisterCaravan(class ACaravan* Caravan);

	// 상단 등록 해제
	UFUNCTION(BlueprintCallable, Category = "Trading Post|Caravan")
	void UnregisterCaravan(class ACaravan* Caravan);

	// === Trade Settings ===

	// 자동 교역 활성화
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading Post|Settings")
	bool bAutoTrade;

	// 자동 교역 간격 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading Post|Settings")
	float AutoTradeInterval;

	// 자동 교역 타이머
	float AutoTradeTimer;

	// 자동 교역 처리
	void ProcessAutoTrade();

	// === Helper Functions ===

	// 교역 가능 여부 확인 (연결, 적대 관계 등)
	UFUNCTION(BlueprintCallable, Category = "Trading Post")
	bool CanTradeWith(ATradingPost* Other) const;

	// 다른 교역소까지의 거리 계산
	UFUNCTION(BlueprintCallable, Category = "Trading Post")
	float GetTradingPostDistance(ATradingPost* Other) const;
};
