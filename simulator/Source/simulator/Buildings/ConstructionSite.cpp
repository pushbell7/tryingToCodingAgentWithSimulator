// Copyright Epic Games, Inc. All Rights Reserved.

#include "ConstructionSite.h"
#include "BaseBuilding.h"
#include "Components/StaticMeshComponent.h"
#include "BuildingManagerSubsystem.h"

AConstructionSite::AConstructionSite()
{
	PrimaryActorTick.bCanEverTick = true;

	// 메시 컴포넌트 생성
	ConstructionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ConstructionMesh"));
	RootComponent = ConstructionMesh;

	// 기본값 설정
	RequiredWorkAmount = 100.0f;
	CurrentWorkAmount = 0.0f;
	MaxWorkers = 5;
	CurrentWorkers = 0;
	WorkRadius = 300.0f;
	bIsActive = true;
	ConstructionStartTime = 0.0f;
	BuildingName = TEXT("Construction Site");
}

void AConstructionSite::BeginPlay()
{
	Super::BeginPlay();

	ConstructionLocation = GetActorLocation();
	ConstructionStartTime = GetWorld()->GetTimeSeconds();

	UE_LOG(LogTemp, Log, TEXT("ConstructionSite created: %s at %s - Required work: %.0f"),
		*BuildingName, *ConstructionLocation.ToString(), RequiredWorkAmount);
}

void AConstructionSite::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 건설 완료 확인
	if (bIsActive && IsConstructionComplete())
	{
		UE_LOG(LogTemp, Warning, TEXT("Construction complete: %s (%.1f seconds)"),
			*BuildingName, GetWorld()->GetTimeSeconds() - ConstructionStartTime);

		// 실제 건물로 교체
		CompleteConstruction();
	}
}

bool AConstructionSite::PerformWork(float WorkAmount)
{
	if (!bIsActive || IsConstructionComplete())
	{
		return false;
	}

	CurrentWorkAmount += WorkAmount;

	// 진행도에 따라 비주얼 업데이트
	UpdateConstructionVisual();

	UE_LOG(LogTemp, Log, TEXT("ConstructionSite %s: Work performed %.1f (%.1f%%)"),
		*BuildingName, WorkAmount, GetConstructionProgress() * 100.0f);

	return true;
}

void AConstructionSite::RegisterWorker()
{
	if (CurrentWorkers < MaxWorkers)
	{
		CurrentWorkers++;
		UE_LOG(LogTemp, Log, TEXT("ConstructionSite %s: Worker registered (%d/%d)"),
			*BuildingName, CurrentWorkers, MaxWorkers);
	}
}

void AConstructionSite::UnregisterWorker()
{
	if (CurrentWorkers > 0)
	{
		CurrentWorkers--;
		UE_LOG(LogTemp, Log, TEXT("ConstructionSite %s: Worker left (%d/%d)"),
			*BuildingName, CurrentWorkers, MaxWorkers);
	}
}

bool AConstructionSite::IsConstructionComplete() const
{
	return CurrentWorkAmount >= RequiredWorkAmount;
}

float AConstructionSite::GetConstructionProgress() const
{
	if (RequiredWorkAmount <= 0.0f)
	{
		return 1.0f;
	}
	return FMath::Clamp(CurrentWorkAmount / RequiredWorkAmount, 0.0f, 1.0f);
}

bool AConstructionSite::HasAvailableWorkerSlots() const
{
	return bIsActive && CurrentWorkers < MaxWorkers && !IsConstructionComplete();
}

ABaseBuilding* AConstructionSite::CompleteConstruction()
{
	if (!BuildingClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ConstructionSite %s: No BuildingClass specified!"), *BuildingName);
		return nullptr;
	}

	// 실제 건물 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ABaseBuilding* NewBuilding = GetWorld()->SpawnActor<ABaseBuilding>(
		BuildingClass,
		ConstructionLocation,
		GetActorRotation(),
		SpawnParams
	);

	if (NewBuilding)
	{
		UE_LOG(LogTemp, Warning, TEXT("Building constructed: %s at %s"),
			*NewBuilding->BuildingName, *ConstructionLocation.ToString());

		// BuildingManagerSubsystem에 등록
		UBuildingManagerSubsystem* BuildingManager = GetWorld()->GetSubsystem<UBuildingManagerSubsystem>();
		if (BuildingManager)
		{
			BuildingManager->RegisterBuilding(NewBuilding);
		}

		// 건설 현장 비활성화 및 제거
		bIsActive = false;
		Destroy();

		return NewBuilding;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn building for %s"), *BuildingName);
		return nullptr;
	}
}

void AConstructionSite::UpdateConstructionVisual()
{
	// 진행도에 따라 메시의 스케일이나 머티리얼 변경 가능
	// 현재는 로그만 출력
	float Progress = GetConstructionProgress();

	if (ConstructionMesh)
	{
		// 높이를 진행도에 따라 조정 (0%: 10%, 100%: 100%)
		float ScaleZ = 0.1f + (0.9f * Progress);
		FVector CurrentScale = ConstructionMesh->GetRelativeScale3D();
		ConstructionMesh->SetRelativeScale3D(FVector(CurrentScale.X, CurrentScale.Y, ScaleZ));
	}
}
