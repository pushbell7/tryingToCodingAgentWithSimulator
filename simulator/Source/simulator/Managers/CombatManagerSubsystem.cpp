// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatManagerSubsystem.h"
#include "CombatEncounter.h"
#include "MilitaryUnit.h"
#include "Engine/World.h"

void UCombatManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("CombatManagerSubsystem initialized"));

	// 기본 설정
	bAutoStartCombat = true;
	CollisionDetectionRadius = 500.0f;
	CollisionCheckInterval = 1.0f;
	CollisionCheckTimer = 0.0f;
}

void UCombatManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	// 모든 전투 종료
	for (ACombatEncounter* Combat : ActiveCombats)
	{
		if (Combat)
		{
			Combat->EndCombat();
		}
	}

	ActiveCombats.Empty();
	RegisteredUnits.Empty();

	UE_LOG(LogTemp, Log, TEXT("CombatManagerSubsystem deinitialized"));
}

void UCombatManagerSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 완료된 전투 정리
	CleanupFinishedCombats();

	// 자동 전투 시작이 활성화되어 있으면 충돌 체크
	if (bAutoStartCombat)
	{
		CollisionCheckTimer += DeltaTime;

		if (CollisionCheckTimer >= CollisionCheckInterval)
		{
			CollisionCheckTimer = 0.0f;
			CheckUnitCollisions();
		}
	}
}

TStatId UCombatManagerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCombatManagerSubsystem, STATGROUP_Tickables);
}

ACombatEncounter* UCombatManagerSubsystem::StartCombat(TArray<AMilitaryUnit*> Units, FVector Location)
{
	if (Units.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start combat with less than 2 units"));
		return nullptr;
	}

	// 이미 해당 위치에서 전투가 진행 중인지 확인
	ACombatEncounter* ExistingCombat = GetCombatAtLocation(Location, CollisionDetectionRadius);
	if (ExistingCombat)
	{
		// 기존 전투에 부대 추가
		for (AMilitaryUnit* Unit : Units)
		{
			if (!IsUnitInCombat(Unit))
			{
				ExistingCombat->AddUnit(Unit);
			}
		}
		return ExistingCombat;
	}

	// 새로운 전투 생성
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACombatEncounter* NewCombat = World->SpawnActor<ACombatEncounter>(
		ACombatEncounter::StaticClass(),
		Location,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (NewCombat)
	{
		NewCombat->StartCombat(Units, Location);
		ActiveCombats.Add(NewCombat);

		UE_LOG(LogTemp, Log, TEXT("Started new combat at %s with %d units"),
			*Location.ToString(), Units.Num());
	}

	return NewCombat;
}

void UCombatManagerSubsystem::EndCombat(ACombatEncounter* Combat)
{
	if (Combat)
	{
		Combat->EndCombat();
		ActiveCombats.Remove(Combat);
	}
}

ACombatEncounter* UCombatManagerSubsystem::GetCombatAtLocation(FVector Location, float Radius) const
{
	for (ACombatEncounter* Combat : ActiveCombats)
	{
		if (Combat && Combat->IsCombatActive())
		{
			float Distance = FVector::Dist(Combat->CombatLocation, Location);
			if (Distance <= Radius)
			{
				return Combat;
			}
		}
	}
	return nullptr;
}

bool UCombatManagerSubsystem::IsUnitInCombat(AMilitaryUnit* Unit) const
{
	return FindCombatForUnit(Unit) != nullptr;
}

ACombatEncounter* UCombatManagerSubsystem::FindCombatForUnit(AMilitaryUnit* Unit) const
{
	if (!Unit) return nullptr;

	for (ACombatEncounter* Combat : ActiveCombats)
	{
		if (Combat && Combat->IsCombatActive())
		{
			for (const FCombatParticipant& P : Combat->Participants)
			{
				if (P.Unit == Unit)
				{
					return Combat;
				}
			}
		}
	}
	return nullptr;
}

TArray<AMilitaryUnit*> UCombatManagerSubsystem::FindHostileUnitsAtLocation(
	AMilitaryUnit* Unit, FVector Location, float Radius) const
{
	TArray<AMilitaryUnit*> HostileUnits;

	if (!Unit) return HostileUnits;

	for (AMilitaryUnit* OtherUnit : RegisteredUnits)
	{
		if (!OtherUnit || OtherUnit == Unit) continue;

		// 거리 체크
		float Distance = FVector::Dist(OtherUnit->GetActorLocation(), Location);
		if (Distance <= Radius)
		{
			// 적대 관계 체크
			if (AreUnitsHostile(Unit, OtherUnit))
			{
				HostileUnits.Add(OtherUnit);
			}
		}
	}

	return HostileUnits;
}

void UCombatManagerSubsystem::CheckUnitCollisions()
{
	// 모든 등록된 부대를 순회하며 충돌 체크
	for (int32 i = 0; i < RegisteredUnits.Num(); i++)
	{
		AMilitaryUnit* UnitA = RegisteredUnits[i];
		if (!UnitA || IsUnitInCombat(UnitA)) continue;

		for (int32 j = i + 1; j < RegisteredUnits.Num(); j++)
		{
			AMilitaryUnit* UnitB = RegisteredUnits[j];
			if (!UnitB || IsUnitInCombat(UnitB)) continue;

			// 적대 관계 체크
			if (!AreUnitsHostile(UnitA, UnitB)) continue;

			// 거리 체크
			float Distance = FVector::Dist(UnitA->GetActorLocation(), UnitB->GetActorLocation());
			if (Distance <= CollisionDetectionRadius)
			{
				// 전투 시작!
				FVector CombatLocation = (UnitA->GetActorLocation() + UnitB->GetActorLocation()) / 2.0f;
				TArray<AMilitaryUnit*> CombatUnits = { UnitA, UnitB };

				UE_LOG(LogTemp, Warning, TEXT("Unit collision detected! Starting combat between %s and %s"),
					*UnitA->UnitName, *UnitB->UnitName);

				StartCombat(CombatUnits, CombatLocation);
			}
		}
	}
}

void UCombatManagerSubsystem::CleanupFinishedCombats()
{
	for (int32 i = ActiveCombats.Num() - 1; i >= 0; i--)
	{
		ACombatEncounter* Combat = ActiveCombats[i];

		if (!Combat || !Combat->IsCombatActive())
		{
			ActiveCombats.RemoveAt(i);
		}
	}
}

bool UCombatManagerSubsystem::AreUnitsHostile(AMilitaryUnit* UnitA, AMilitaryUnit* UnitB) const
{
	// TODO: 실제 팩션 시스템 구현 시 변경
	// 현재는 모든 부대를 적대적으로 간주 (테스트용)
	return true;
}

void UCombatManagerSubsystem::RegisterUnit(AMilitaryUnit* Unit)
{
	if (Unit && !RegisteredUnits.Contains(Unit))
	{
		RegisteredUnits.Add(Unit);
		UE_LOG(LogTemp, Log, TEXT("Unit registered for combat detection: %s"), *Unit->UnitName);
	}
}

void UCombatManagerSubsystem::UnregisterUnit(AMilitaryUnit* Unit)
{
	if (Unit)
	{
		RegisteredUnits.Remove(Unit);

		// 해당 부대가 참가 중인 전투에서 제거
		ACombatEncounter* Combat = FindCombatForUnit(Unit);
		if (Combat)
		{
			Combat->RemoveUnit(Unit);
		}

		UE_LOG(LogTemp, Log, TEXT("Unit unregistered from combat detection: %s"), *Unit->UnitName);
	}
}
