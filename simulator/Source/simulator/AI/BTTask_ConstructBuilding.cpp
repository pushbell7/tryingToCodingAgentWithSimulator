// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_ConstructBuilding.h"
#include "AIController.h"
#include "BaseVillager.h"
#include "ConstructionSite.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EngineUtils.h"

UBTTask_ConstructBuilding::UBTTask_ConstructBuilding()
{
	NodeName = "Construct Building";
	bNotifyTick = true;
	bNotifyTaskFinished = true;

	MaxSearchDistance = 5000.0f;
	WorkRadius = 300.0f;
	WorkRatePerSecond = 5.0f;
	MaxWorkDuration = 30.0f;
	ConstructionSiteKey = FName("ConstructionSite");

	WorkedTime = 0.0f;
	CurrentSite = nullptr;
}

EBTNodeResult::Type UBTTask_ConstructBuilding::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ABaseVillager* Villager = Cast<ABaseVillager>(AIController->GetPawn());
	if (!Villager)
	{
		UE_LOG(LogTemp, Warning, TEXT("ConstructBuilding: No villager pawn"));
		return EBTNodeResult::Failed;
	}

	// 가장 가까운 건설 현장 찾기
	CurrentSite = FindNearestConstructionSite(Villager);

	if (!CurrentSite)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: No construction site available"), *Villager->GetName());
		return EBTNodeResult::Failed;
	}

	// 건설 현장까지의 거리 확인
	float Distance = FVector::Dist(Villager->GetActorLocation(), CurrentSite->GetConstructionLocation());

	if (Distance > WorkRadius)
	{
		// 블랙보드에 건설 현장 저장 (이동 태스크용)
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsObject(ConstructionSiteKey, CurrentSite);
		}

		UE_LOG(LogTemp, Log, TEXT("%s: Not close enough to construction site, need to move (%.0f > %.0f)"),
			*Villager->GetName(), Distance, WorkRadius);
		return EBTNodeResult::Failed; // 이동 필요
	}

	// 작업 슬롯 확인
	if (!CurrentSite->HasAvailableWorkerSlots())
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Construction site %s is full"),
			*Villager->GetName(), *CurrentSite->BuildingName);
		return EBTNodeResult::Failed;
	}

	// 작업자 등록
	CurrentSite->RegisterWorker();

	// 작업 시작
	WorkedTime = 0.0f;
	Villager->CurrentState = EActorState::WORKING;

	UE_LOG(LogTemp, Log, TEXT("%s: Started construction work at %s (Progress: %.1f%%)"),
		*Villager->GetName(), *CurrentSite->BuildingName, CurrentSite->GetConstructionProgress() * 100.0f);

	return EBTNodeResult::InProgress;
}

void UBTTask_ConstructBuilding::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ABaseVillager* Villager = Cast<ABaseVillager>(AIController->GetPawn());
	if (!Villager || !CurrentSite)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 건설 현장이 완료되었거나 비활성화됨
	if (CurrentSite->IsConstructionComplete() || !CurrentSite->bIsActive)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Construction completed or site inactive"), *Villager->GetName());
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 작업 수행
	if (!PerformConstructionWork(CurrentSite, Villager, DeltaSeconds))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 최대 작업 시간 체크
	WorkedTime += DeltaSeconds;
	if (WorkedTime >= MaxWorkDuration)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Max work duration reached (%.1f seconds)"),
			*Villager->GetName(), WorkedTime);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}

void UBTTask_ConstructBuilding::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	// 작업자 등록 해제
	if (CurrentSite)
	{
		CurrentSite->UnregisterWorker();
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		ABaseVillager* Villager = Cast<ABaseVillager>(AIController->GetPawn());
		if (Villager)
		{
			if (TaskResult == EBTNodeResult::Succeeded)
			{
				UE_LOG(LogTemp, Log, TEXT("%s: Construction work completed"), *Villager->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s: Construction work failed"), *Villager->GetName());
			}

			Villager->CurrentState = EActorState::IDLE;
		}
	}

	WorkedTime = 0.0f;
	CurrentSite = nullptr;

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

AConstructionSite* UBTTask_ConstructBuilding::FindNearestConstructionSite(ABaseVillager* Villager)
{
	if (!Villager || !Villager->GetWorld())
	{
		return nullptr;
	}

	AConstructionSite* NearestSite = nullptr;
	float NearestDistance = FLT_MAX;

	// 월드의 모든 건설 현장 검색
	for (TActorIterator<AConstructionSite> It(Villager->GetWorld()); It; ++It)
	{
		AConstructionSite* Site = *It;
		if (Site && Site->bIsActive && Site->HasAvailableWorkerSlots())
		{
			float Distance = FVector::Dist(Villager->GetActorLocation(), Site->GetConstructionLocation());
			if (Distance < NearestDistance && Distance <= MaxSearchDistance)
			{
				NearestDistance = Distance;
				NearestSite = Site;
			}
		}
	}

	return NearestSite;
}

bool UBTTask_ConstructBuilding::PerformConstructionWork(AConstructionSite* Site, ABaseVillager* Villager, float DeltaTime)
{
	if (!Site || !Villager)
	{
		return false;
	}

	// 거리 확인
	float Distance = FVector::Dist(Villager->GetActorLocation(), Site->GetConstructionLocation());
	if (Distance > WorkRadius)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Too far from construction site (%.0f > %.0f)"),
			*Villager->GetName(), Distance, WorkRadius);
		return false;
	}

	// 작업량 계산 (시민의 능력치에 따라 조정 가능)
	float WorkAmount = WorkRatePerSecond * DeltaTime;

	// 작업 수행
	bool bSuccess = Site->PerformWork(WorkAmount);

	if (bSuccess)
	{
		// 에너지 소모 (BaseVillager에 Energy 시스템이 있다면)
		// Villager->ConsumeEnergy(WorkAmount * 0.1f);
	}

	return bSuccess;
}
