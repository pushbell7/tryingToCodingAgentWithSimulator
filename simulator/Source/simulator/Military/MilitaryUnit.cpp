// Copyright Epic Games, Inc. All Rights Reserved.

#include "MilitaryUnit.h"
#include "SoldierVillager.h"
#include "AIController.h"

AMilitaryUnit::AMilitaryUnit()
{
	PrimaryActorTick.bCanEverTick = true;

	UnitName = TEXT("Military Unit");
	MaxUnitSize = 50;
	Commander = nullptr;

	CurrentFormation = EFormationType::Line;
	FormationSpacing = 100.0f;
	FormationCenter = FVector::ZeroVector;
	FormationRotation = FRotator::ZeroRotator;

	TargetLocation = FVector::ZeroVector;
	bIsMoving = false;
	MovementSpeed = 300.0f;
}

void AMilitaryUnit::BeginPlay()
{
	Super::BeginPlay();

	FormationCenter = GetActorLocation();
	FormationRotation = GetActorRotation();

	UE_LOG(LogTemp, Log, TEXT("MilitaryUnit %s created at %s"), *UnitName, *FormationCenter.ToString());
}

void AMilitaryUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 부대 중심 업데이트
	UpdateFormationCenter();

	// 이동 중이면 이동 처리
	if (bIsMoving)
	{
		UpdateMovement(DeltaTime);
	}

	// 대형 유지
	MaintainFormation(DeltaTime);
}

bool AMilitaryUnit::AddSoldier(ASoldierVillager* Soldier, int32 Position)
{
	if (!Soldier)
	{
		return false;
	}

	// 정원 확인
	if (Soldiers.Num() >= MaxUnitSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unit %s: Max unit size reached (%d)"), *UnitName, MaxUnitSize);
		return false;
	}

	// 이미 소속되어 있는지 확인
	if (Soldiers.Contains(Soldier))
	{
		UE_LOG(LogTemp, Warning, TEXT("Unit %s: %s already in unit"), *UnitName, *Soldier->VillagerName);
		return false;
	}

	// 부대에 추가
	if (Position >= 0 && Position < Soldiers.Num())
	{
		Soldiers.Insert(Soldier, Position);
	}
	else
	{
		Soldiers.Add(Soldier);
	}

	// 병사에게 부대 정보 설정
	Soldier->AssignToUnit(this, Soldiers.Num() - 1);

	// 첫 번째 병사를 지휘관으로 설정
	if (!Commander && Soldiers.Num() > 0)
	{
		Commander = Soldiers[0];
	}

	UE_LOG(LogTemp, Log, TEXT("Unit %s: Added %s (%d/%d)"),
		*UnitName, *Soldier->VillagerName, Soldiers.Num(), MaxUnitSize);

	return true;
}

bool AMilitaryUnit::RemoveSoldier(ASoldierVillager* Soldier)
{
	if (!Soldier || !Soldiers.Contains(Soldier))
	{
		return false;
	}

	Soldiers.Remove(Soldier);
	Soldier->UnassignFromUnit();

	// 지휘관이 제거되면 새로 지정
	if (Commander == Soldier)
	{
		Commander = Soldiers.Num() > 0 ? Soldiers[0] : nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("Unit %s: Removed %s (%d/%d)"),
		*UnitName, *Soldier->VillagerName, Soldiers.Num(), MaxUnitSize);

	return true;
}

void AMilitaryUnit::DisbandUnit()
{
	UE_LOG(LogTemp, Warning, TEXT("Unit %s: Disbanding unit with %d soldiers"), *UnitName, Soldiers.Num());

	for (ASoldierVillager* Soldier : Soldiers)
	{
		if (Soldier)
		{
			Soldier->UnassignFromUnit();
		}
	}

	Soldiers.Empty();
	Commander = nullptr;

	// 부대 액터 제거
	Destroy();
}

void AMilitaryUnit::SetFormation(EFormationType NewFormation)
{
	if (CurrentFormation == NewFormation)
	{
		return;
	}

	CurrentFormation = NewFormation;

	UE_LOG(LogTemp, Log, TEXT("Unit %s: Formation changed to %d"), *UnitName, (int32)NewFormation);

	// 대형 즉시 적용
	MaintainFormation(0.0f);
}

void AMilitaryUnit::MoveToLocation(FVector Location)
{
	TargetLocation = Location;
	bIsMoving = true;

	UE_LOG(LogTemp, Log, TEXT("Unit %s: Moving to %s"), *UnitName, *Location.ToString());
}

void AMilitaryUnit::StopMovement()
{
	bIsMoving = false;

	UE_LOG(LogTemp, Log, TEXT("Unit %s: Stopped movement"), *UnitName);
}

FVector AMilitaryUnit::GetFormationPosition(int32 SoldierIndex) const
{
	if (SoldierIndex < 0 || SoldierIndex >= Soldiers.Num())
	{
		return FormationCenter;
	}

	switch (CurrentFormation)
	{
	case EFormationType::Line:
		return CalculateLineFormationPosition(SoldierIndex);
	case EFormationType::Column:
		return CalculateColumnFormationPosition(SoldierIndex);
	case EFormationType::Box:
		return CalculateBoxFormationPosition(SoldierIndex);
	case EFormationType::Wedge:
		return CalculateWedgeFormationPosition(SoldierIndex);
	case EFormationType::Scatter:
		return CalculateScatterFormationPosition(SoldierIndex);
	default:
		return FormationCenter;
	}
}

void AMilitaryUnit::UpdateFormationCenter()
{
	if (Soldiers.Num() == 0)
	{
		return;
	}

	// 모든 병사의 평균 위치를 중심으로
	FVector TotalPosition = FVector::ZeroVector;
	int32 ValidCount = 0;

	for (ASoldierVillager* Soldier : Soldiers)
	{
		if (Soldier)
		{
			TotalPosition += Soldier->GetActorLocation();
			ValidCount++;
		}
	}

	if (ValidCount > 0)
	{
		FormationCenter = TotalPosition / ValidCount;
		SetActorLocation(FormationCenter);
	}
}

int32 AMilitaryUnit::GetCombatReadyCount() const
{
	int32 Count = 0;
	for (ASoldierVillager* Soldier : Soldiers)
	{
		if (Soldier && Soldier->CanFight())
		{
			Count++;
		}
	}
	return Count;
}

float AMilitaryUnit::GetAverageHealth() const
{
	if (Soldiers.Num() == 0)
	{
		return 0.0f;
	}

	float TotalHealth = 0.0f;
	for (ASoldierVillager* Soldier : Soldiers)
	{
		if (Soldier)
		{
			TotalHealth += Soldier->CurrentHealth;
		}
	}

	return TotalHealth / Soldiers.Num();
}

void AMilitaryUnit::MaintainFormation(float DeltaTime)
{
	// 각 병사를 대형 위치로 이동
	for (int32 i = 0; i < Soldiers.Num(); i++)
	{
		ASoldierVillager* Soldier = Soldiers[i];
		if (!Soldier)
		{
			continue;
		}

		FVector TargetPos = GetFormationPosition(i);
		FVector CurrentPos = Soldier->GetActorLocation();

		float Distance = FVector::Dist(CurrentPos, TargetPos);

		// 대형 위치에서 너무 멀어지면 이동
		if (Distance > FormationSpacing * 0.5f)
		{
			// AIController를 통해 이동
			AAIController* AIController = Cast<AAIController>(Soldier->GetController());
			if (AIController)
			{
				AIController->MoveToLocation(TargetPos, FormationSpacing * 0.3f);
			}
		}
	}
}

void AMilitaryUnit::UpdateMovement(float DeltaTime)
{
	float DistanceToTarget = FVector::Dist(FormationCenter, TargetLocation);

	// 목표 도착
	if (DistanceToTarget < FormationSpacing)
	{
		StopMovement();
		return;
	}

	// 부대 중심을 목표로 이동
	FVector Direction = (TargetLocation - FormationCenter).GetSafeNormal();
	FVector NewCenter = FormationCenter + (Direction * MovementSpeed * DeltaTime);

	FormationCenter = NewCenter;
	FormationRotation = Direction.Rotation();
}

FVector AMilitaryUnit::CalculateLineFormationPosition(int32 Index) const
{
	// 횡대: 일렬로 배치
	int32 HalfSize = Soldiers.Num() / 2;
	int32 Offset = Index - HalfSize;

	FVector RightVector = FormationRotation.RotateVector(FVector::RightVector);
	return FormationCenter + (RightVector * Offset * FormationSpacing);
}

FVector AMilitaryUnit::CalculateColumnFormationPosition(int32 Index) const
{
	// 종대: 세로로 배치
	FVector ForwardVector = FormationRotation.RotateVector(FVector::ForwardVector);
	return FormationCenter - (ForwardVector * Index * FormationSpacing);
}

FVector AMilitaryUnit::CalculateBoxFormationPosition(int32 Index) const
{
	// 방진: 사각형으로 배치
	int32 SideLength = FMath::CeilToInt(FMath::Sqrt((float)Soldiers.Num()));
	int32 Row = Index / SideLength;
	int32 Col = Index % SideLength;

	FVector ForwardVector = FormationRotation.RotateVector(FVector::ForwardVector);
	FVector RightVector = FormationRotation.RotateVector(FVector::RightVector);

	return FormationCenter
		- (ForwardVector * Row * FormationSpacing)
		+ (RightVector * (Col - SideLength / 2) * FormationSpacing);
}

FVector AMilitaryUnit::CalculateWedgeFormationPosition(int32 Index) const
{
	// 쐐기: V자 형태
	int32 Row = 0;
	int32 IndexInRow = Index;

	// 행 계산
	for (int32 i = 1; IndexInRow >= i; i++)
	{
		IndexInRow -= i;
		Row++;
	}

	int32 Offset = IndexInRow - (Row / 2);

	FVector ForwardVector = FormationRotation.RotateVector(FVector::ForwardVector);
	FVector RightVector = FormationRotation.RotateVector(FVector::RightVector);

	return FormationCenter
		- (ForwardVector * Row * FormationSpacing)
		+ (RightVector * Offset * FormationSpacing);
}

FVector AMilitaryUnit::CalculateScatterFormationPosition(int32 Index) const
{
	// 산개: 넓게 퍼진 형태
	int32 SideLength = FMath::CeilToInt(FMath::Sqrt((float)Soldiers.Num()));
	int32 Row = Index / SideLength;
	int32 Col = Index % SideLength;

	float ScatterSpacing = FormationSpacing * 2.0f; // 더 넓은 간격

	FVector ForwardVector = FormationRotation.RotateVector(FVector::ForwardVector);
	FVector RightVector = FormationRotation.RotateVector(FVector::RightVector);

	return FormationCenter
		- (ForwardVector * Row * ScatterSpacing)
		+ (RightVector * (Col - SideLength / 2) * ScatterSpacing);
}
