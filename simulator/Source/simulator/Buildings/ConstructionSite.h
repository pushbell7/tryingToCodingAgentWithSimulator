// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimulatorTypes.h"
#include "ConstructionSite.generated.h"

/**
 * 건설 중인 건물을 나타내는 액터
 * 시민들이 작업하여 일정 작업량을 채우면 실제 건물로 교체됨
 */
UCLASS()
class SIMULATOR_API AConstructionSite : public AActor
{
	GENERATED_BODY()

public:
	AConstructionSite();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 건설할 건물 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	EBuildingType BuildingType;

	// 건설할 건물의 블루프린트 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	TSubclassOf<class ABaseBuilding> BuildingClass;

	// 건설에 필요한 총 작업량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	float RequiredWorkAmount;

	// 현재 완료된 작업량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Construction")
	float CurrentWorkAmount;

	// 최대 동시 작업 인원
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	int32 MaxWorkers;

	// 현재 작업 중인 인원
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Construction")
	int32 CurrentWorkers;

	// 작업 반경 (이 거리 안에서 작업 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	float WorkRadius;

	// 건설 위치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Construction")
	FVector ConstructionLocation;

	// 건설 중인지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Construction")
	bool bIsActive;

	// 건설 시작한 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Construction")
	float ConstructionStartTime;

	// 건물 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	FString BuildingName;

	// 메시 컴포넌트 (건설 중 비주얼)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ConstructionMesh;

	/**
	 * 작업자가 작업을 수행
	 * @param WorkAmount 수행한 작업량
	 * @return 작업이 성공적으로 적용되었는지 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Construction")
	bool PerformWork(float WorkAmount);

	/**
	 * 작업자가 건설 현장에 도착
	 */
	UFUNCTION(BlueprintCallable, Category = "Construction")
	void RegisterWorker();

	/**
	 * 작업자가 건설 현장에서 이탈
	 */
	UFUNCTION(BlueprintCallable, Category = "Construction")
	void UnregisterWorker();

	/**
	 * 건설이 완료되었는지 확인
	 */
	UFUNCTION(BlueprintCallable, Category = "Construction")
	bool IsConstructionComplete() const;

	/**
	 * 건설 진행률 (0.0 ~ 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Construction")
	float GetConstructionProgress() const;

	/**
	 * 작업 가능한 슬롯이 있는지 확인
	 */
	UFUNCTION(BlueprintCallable, Category = "Construction")
	bool HasAvailableWorkerSlots() const;

	/**
	 * 건설 완료 시 실제 건물 생성
	 */
	UFUNCTION(BlueprintCallable, Category = "Construction")
	class ABaseBuilding* CompleteConstruction();

	/**
	 * 건설 위치 반환
	 */
	UFUNCTION(BlueprintCallable, Category = "Construction")
	FVector GetConstructionLocation() const { return ConstructionLocation; }

private:
	/**
	 * 건설 진행도에 따라 비주얼 업데이트
	 */
	void UpdateConstructionVisual();
};
