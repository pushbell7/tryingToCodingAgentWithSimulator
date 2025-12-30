// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatEncounter.h"
#include "MilitaryUnit.h"
#include "SoldierVillager.h"
#include "Particles/ParticleSystemComponent.h"

ACombatEncounter::ACombatEncounter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 기본 설정
	CombatState = ECombatState::Inactive;
	TurnCount = 0;
	TurnTimer = 0.0f;

	// 전투 설정
	CombatTurnInterval = 3.0f;  // 3초마다 턴 처리
	MoraleDecayRate = 0.05f;    // 사상자 비율당 5% 사기 감소
	RetreatMoraleThreshold = 0.3f; // 사기 30% 이하면 후퇴
	EffectScale = 1.0f;

	// 전투 이펙트 컴포넌트 생성
	CombatEffectComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("CombatEffect"));
	RootComponent = CombatEffectComponent;
	CombatEffectComponent->bAutoActivate = false;
}

void ACombatEncounter::BeginPlay()
{
	Super::BeginPlay();
}

void ACombatEncounter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CombatState == ECombatState::Engaged)
	{
		// 턴 타이머 업데이트
		TurnTimer += DeltaTime;

		if (TurnTimer >= CombatTurnInterval)
		{
			TurnTimer = 0.0f;
			ProcessCombatTurn();
		}

		// 이펙트 업데이트
		UpdateCombatEffect();
	}
}

void ACombatEncounter::StartCombat(TArray<AMilitaryUnit*> Units, FVector Location)
{
	if (Units.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start combat with less than 2 units"));
		return;
	}

	CombatLocation = Location;
	SetActorLocation(Location);

	// 참가자 초기화
	Participants.Empty();
	for (AMilitaryUnit* Unit : Units)
	{
		if (Unit)
		{
			int32 Strength = Unit->GetCombatReadyCount();
			if (Strength > 0)
			{
				Participants.Add(FCombatParticipant(Unit, Strength));
			}
		}
	}

	if (Participants.Num() >= 2)
	{
		CombatState = ECombatState::Engaged;
		TurnCount = 0;
		TurnTimer = 0.0f;

		// 전투 이펙트 활성화
		if (CombatEffectComponent)
		{
			CombatEffectComponent->Activate(true);
		}

		// 모든 참가 부대에게 전투 진입 알림
		for (FCombatParticipant& P : Participants)
		{
			if (P.Unit)
			{
				P.Unit->EnterCombat(this);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("Combat started at location %s with %d units"),
			*Location.ToString(), Participants.Num());
	}
}

bool ACombatEncounter::AddUnit(AMilitaryUnit* Unit)
{
	if (!Unit || CombatState != ECombatState::Engaged)
		return false;

	// 이미 참가 중인지 확인
	if (FindParticipant(Unit))
		return false;

	int32 Strength = Unit->GetCombatReadyCount();
	if (Strength > 0)
	{
		Participants.Add(FCombatParticipant(Unit, Strength));
		Unit->EnterCombat(this); // 전투 진입 알림
		UE_LOG(LogTemp, Log, TEXT("Unit joined combat with %d soldiers"), Strength);
		return true;
	}

	return false;
}

bool ACombatEncounter::RemoveUnit(AMilitaryUnit* Unit)
{
	for (int32 i = 0; i < Participants.Num(); i++)
	{
		if (Participants[i].Unit == Unit)
		{
			// 전투 이탈 알림
			if (Unit)
			{
				Unit->LeaveCombat();
			}

			Participants.RemoveAt(i);

			// 1개 이하로 줄어들면 전투 종료
			if (Participants.Num() <= 1)
			{
				EndCombat();
			}

			return true;
		}
	}
	return false;
}

void ACombatEncounter::EndCombat()
{
	CombatState = ECombatState::Finished;

	// 모든 참가 부대에게 전투 이탈 알림
	for (FCombatParticipant& P : Participants)
	{
		if (P.Unit)
		{
			P.Unit->LeaveCombat();
		}
	}

	// 이펙트 비활성화
	if (CombatEffectComponent)
	{
		CombatEffectComponent->Deactivate();
	}

	// 승자 로그
	AMilitaryUnit* Winner = GetWinner();
	if (Winner)
	{
		UE_LOG(LogTemp, Log, TEXT("Combat ended. Winner: %s"), *Winner->UnitName);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Combat ended with no clear winner"));
	}

	// 일정 시간 후 액터 제거
	SetLifeSpan(5.0f);
}

int32 ACombatEncounter::GetTotalCombatants() const
{
	int32 Total = 0;
	for (const FCombatParticipant& P : Participants)
	{
		Total += P.CurrentStrength;
	}
	return Total;
}

AMilitaryUnit* ACombatEncounter::GetWinner() const
{
	if (Participants.Num() == 1 && Participants[0].CurrentStrength > 0)
	{
		return Participants[0].Unit;
	}
	return nullptr;
}

void ACombatEncounter::ProcessCombatTurn()
{
	TurnCount++;

	UE_LOG(LogTemp, Log, TEXT("=== Combat Turn %d ==="), TurnCount);

	// 1. 전투 피해 계산
	CalculateCombatDamage();

	// 2. 사기 업데이트
	UpdateMorale();

	// 3. 후퇴 체크
	CheckRetreat();

	// 4. 전투 종료 조건 체크
	if (CheckCombatEnd())
	{
		EndCombat();
	}
}

void ACombatEncounter::CalculateCombatDamage()
{
	// 간단한 전투 모델: 모든 부대가 서로 공격
	// 각 부대의 총 공격력을 계산하고, 다른 부대들에게 분산 피해

	for (int32 i = 0; i < Participants.Num(); i++)
	{
		FCombatParticipant& Attacker = Participants[i];

		if (!Attacker.Unit || Attacker.CurrentStrength <= 0)
			continue;

		// 이 부대의 총 공격력 계산
		float TotalAttackPower = 0.0f;
		float TotalDefensePower = 0.0f;

		for (ASoldierVillager* Soldier : Attacker.Unit->Soldiers)
		{
			if (Soldier && Soldier->CanFight())
			{
				TotalAttackPower += Soldier->AttackPower;
				TotalDefensePower += Soldier->DefensePower;
			}
		}

		// 다른 부대들에게 피해 분산
		int32 EnemyCount = Participants.Num() - 1;
		if (EnemyCount > 0)
		{
			float DamagePerEnemy = TotalAttackPower / EnemyCount;

			for (int32 j = 0; j < Participants.Num(); j++)
			{
				if (i == j) continue;  // 자기 자신은 공격 안 함

				FCombatParticipant& Defender = Participants[j];

				// 방어력 계산
				float DefenderDefense = 0.0f;
				if (Defender.Unit)
				{
					for (ASoldierVillager* Soldier : Defender.Unit->Soldiers)
					{
						if (Soldier && Soldier->CanFight())
						{
							DefenderDefense += Soldier->DefensePower;
						}
					}
				}

				// 실제 피해 = 공격력 - 방어력 (최소 공격력의 10%)
				float ActualDamage = FMath::Max(DamagePerEnemy - (DefenderDefense / Defender.CurrentStrength),
					DamagePerEnemy * 0.1f);

				// 피해를 사상자로 변환 (대략적으로 100 HP = 1명)
				int32 Casualties = FMath::RoundToInt(ActualDamage / 100.0f);
				Casualties = FMath::Max(Casualties, 1); // 최소 1명

				ApplyDamageToUnit(Defender, Casualties);
			}
		}
	}
}

void ACombatEncounter::ApplyDamageToUnit(FCombatParticipant& Participant, int32 Damage)
{
	if (!Participant.Unit || Participant.CurrentStrength <= 0)
		return;

	// 사상자 적용
	int32 ActualCasualties = FMath::Min(Damage, Participant.CurrentStrength);
	Participant.CurrentStrength -= ActualCasualties;
	Participant.TotalCasualties += ActualCasualties;

	// 부대의 병사들에게 실제 피해 적용
	int32 RemainingCasualties = ActualCasualties;
	TArray<ASoldierVillager*>& Soldiers = Participant.Unit->Soldiers;

	for (int32 i = Soldiers.Num() - 1; i >= 0 && RemainingCasualties > 0; i--)
	{
		ASoldierVillager* Soldier = Soldiers[i];
		if (Soldier && Soldier->CanFight())
		{
			// 병사에게 치명적 피해 (즉사 또는 중상)
			float FatalDamage = Soldier->MaxHealth * 1.5f;
			Soldier->TakeCombatDamage(FatalDamage);
			RemainingCasualties--;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Unit %s took %d casualties. Remaining: %d"),
		*Participant.Unit->UnitName, ActualCasualties, Participant.CurrentStrength);
}

void ACombatEncounter::UpdateMorale()
{
	for (FCombatParticipant& P : Participants)
	{
		if (P.InitialStrength <= 0) continue;

		// 사상자 비율에 따라 사기 감소
		float CasualtyRate = (float)P.TotalCasualties / (float)P.InitialStrength;
		float MoraleDecay = CasualtyRate * MoraleDecayRate;

		P.Morale = FMath::Max(P.Morale - MoraleDecay, 0.0f);

		UE_LOG(LogTemp, Log, TEXT("Unit %s morale: %.2f (casualties: %d/%d)"),
			*P.Unit->UnitName, P.Morale, P.TotalCasualties, P.InitialStrength);
	}
}

void ACombatEncounter::CheckRetreat()
{
	// 사기가 너무 낮은 부대는 후퇴
	for (int32 i = Participants.Num() - 1; i >= 0; i--)
	{
		FCombatParticipant& P = Participants[i];

		if (P.Morale < RetreatMoraleThreshold && P.CurrentStrength > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Unit %s is retreating due to low morale!"),
				*P.Unit->UnitName);

			// TODO: 실제 후퇴 로직 (부대를 전투에서 제거하고 안전한 위치로 이동)
			Participants.RemoveAt(i);
		}
	}
}

bool ACombatEncounter::CheckCombatEnd()
{
	// 전투 종료 조건:
	// 1. 참가 부대가 1개 이하
	// 2. 모든 부대가 전멸

	if (Participants.Num() <= 1)
		return true;

	int32 ActiveUnits = 0;
	for (const FCombatParticipant& P : Participants)
	{
		if (P.CurrentStrength > 0)
			ActiveUnits++;
	}

	return ActiveUnits <= 1;
}

void ACombatEncounter::UpdateCombatEffect()
{
	if (!CombatEffectComponent || !CombatEffectComponent->IsActive())
		return;

	// 전투 병력 수에 따라 이펙트 크기 조정
	int32 TotalCombatants = GetTotalCombatants();
	float Scale = FMath::Lerp(0.5f, 3.0f, FMath::Min(TotalCombatants / 100.0f, 1.0f));

	CombatEffectComponent->SetWorldScale3D(FVector(Scale * EffectScale));
}

FCombatParticipant* ACombatEncounter::FindParticipant(AMilitaryUnit* Unit)
{
	for (FCombatParticipant& P : Participants)
	{
		if (P.Unit == Unit)
			return &P;
	}
	return nullptr;
}
