// Copyright Epic Games, Inc. All Rights Reserved.

#include "ResourceManagerSubsystem.h"
#include "BaseBuilding.h"
#include "InventoryComponent.h"
#include "BuildingManagerSubsystem.h"
#include "TimerManager.h"

void UResourceManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RefreshInterval = 2.0f; // 2초마다 자원 캐시 갱신
	LastRefreshTime = 0.0f;

	// 초기 갱신
	RefreshResourceCache();

	// 주기적 갱신 타이머 설정
	if (RefreshInterval > 0.0f && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			RefreshTimerHandle,
			this,
			&UResourceManagerSubsystem::RefreshResourceCache,
			RefreshInterval,
			true
		);
	}

	UE_LOG(LogTemp, Log, TEXT("ResourceManagerSubsystem initialized"));
	LogResourceStatus();
}

void UResourceManagerSubsystem::Deinitialize()
{
	// 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}

	CachedResourceTotals.Empty();

	Super::Deinitialize();
}

void UResourceManagerSubsystem::RefreshResourceCache()
{
	CachedResourceTotals.Empty();

	if (!GetWorld())
	{
		return;
	}

	// BuildingManagerSubsystem에서 모든 창고 가져오기
	UBuildingManagerSubsystem* BuildingManager = GetWorld()->GetSubsystem<UBuildingManagerSubsystem>();
	if (!BuildingManager)
	{
		return;
	}

	TArray<ABaseBuilding*> StorageBuildings = BuildingManager->GetAllStorageBuildings();

	// 모든 창고의 자원 합산
	for (ABaseBuilding* Building : StorageBuildings)
	{
		if (Building && Building->Inventory)
		{
			TArray<FResourceStack> Resources = Building->Inventory->GetAllResources();
			for (const FResourceStack& Stack : Resources)
			{
				if (!CachedResourceTotals.Contains(Stack.ResourceType))
				{
					CachedResourceTotals.Add(Stack.ResourceType, 0);
				}
				CachedResourceTotals[Stack.ResourceType] += Stack.Quantity;
			}
		}
	}

	LastRefreshTime = GetWorld()->GetTimeSeconds();
}

int32 UResourceManagerSubsystem::GetTotalResource(EResourceType ResourceType) const
{
	const int32* Found = CachedResourceTotals.Find(ResourceType);
	return Found ? *Found : 0;
}

bool UResourceManagerSubsystem::HasEnoughResource(EResourceType ResourceType, int32 RequiredAmount) const
{
	return GetTotalResource(ResourceType) >= RequiredAmount;
}

bool UResourceManagerSubsystem::HasEnoughResources(const TArray<FResourceStack>& RequiredResources) const
{
	for (const FResourceStack& Required : RequiredResources)
	{
		if (!HasEnoughResource(Required.ResourceType, Required.Quantity))
		{
			return false;
		}
	}
	return true;
}

TMap<EResourceType, int32> UResourceManagerSubsystem::GetAllResourceTotals() const
{
	return CachedResourceTotals;
}

bool UResourceManagerSubsystem::DeductResource(EResourceType ResourceType, int32 Amount)
{
	if (!HasEnoughResource(ResourceType, Amount))
	{
		UE_LOG(LogTemp, Warning, TEXT("ResourceManager: Not enough %d (need %d, have %d)"),
			(int32)ResourceType, Amount, GetTotalResource(ResourceType));
		return false;
	}

	if (!GetWorld())
	{
		return false;
	}

	// BuildingManagerSubsystem에서 창고 가져오기
	UBuildingManagerSubsystem* BuildingManager = GetWorld()->GetSubsystem<UBuildingManagerSubsystem>();
	if (!BuildingManager)
	{
		return false;
	}

	TArray<ABaseBuilding*> StorageBuildings = BuildingManager->GetAllStorageBuildings();

	// 창고에서 순차적으로 차감
	int32 RemainingAmount = Amount;
	for (ABaseBuilding* Building : StorageBuildings)
	{
		if (Building && Building->Inventory && RemainingAmount > 0)
		{
			int32 Removed = Building->Inventory->RemoveResource(ResourceType, RemainingAmount);
			RemainingAmount -= Removed;

			if (Removed > 0)
			{
				UE_LOG(LogTemp, Log, TEXT("ResourceManager: Deducted %d x %d from %s"),
					(int32)ResourceType, Removed, *Building->BuildingName);
			}
		}
	}

	// 캐시 갱신
	RefreshResourceCache();

	bool bSuccess = (RemainingAmount == 0);
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("ResourceManager: Failed to deduct full amount (missing %d)"), RemainingAmount);
	}

	return bSuccess;
}

bool UResourceManagerSubsystem::DeductResources(const TArray<FResourceStack>& Resources)
{
	// 먼저 모든 자원이 충분한지 확인
	if (!HasEnoughResources(Resources))
	{
		UE_LOG(LogTemp, Warning, TEXT("ResourceManager: Not enough resources for deduction"));
		LogResourceStatus();
		return false;
	}

	// 모든 자원 차감
	for (const FResourceStack& Stack : Resources)
	{
		if (!DeductResource(Stack.ResourceType, Stack.Quantity))
		{
			// 실패 시 롤백은 복잡하므로, 사전 체크로 방지
			UE_LOG(LogTemp, Error, TEXT("ResourceManager: Deduction failed mid-process!"));
			return false;
		}
	}

	return true;
}

bool UResourceManagerSubsystem::RefundResources(const TArray<FResourceStack>& Resources, FVector Location)
{
	if (!GetWorld())
	{
		return false;
	}

	// BuildingManagerSubsystem에서 가장 가까운 창고 찾기
	UBuildingManagerSubsystem* BuildingManager = GetWorld()->GetSubsystem<UBuildingManagerSubsystem>();
	if (!BuildingManager)
	{
		return false;
	}

	ABaseBuilding* NearestStorage = BuildingManager->GetNearestAvailableStorage(Location);
	if (!NearestStorage || !NearestStorage->Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("ResourceManager: No storage available for refund"));
		return false;
	}

	// 자원 반환
	for (const FResourceStack& Stack : Resources)
	{
		int32 Added = NearestStorage->Inventory->AddResource(Stack.ResourceType, Stack.Quantity);
		if (Added < Stack.Quantity)
		{
			UE_LOG(LogTemp, Warning, TEXT("ResourceManager: Storage full, couldn't refund all resources"));
		}
	}

	// 캐시 갱신
	RefreshResourceCache();

	return true;
}

bool UResourceManagerSubsystem::CanAffordConstruction(const FConstructionCost& Cost) const
{
	return HasEnoughResources(Cost.RequiredResources);
}

bool UResourceManagerSubsystem::PayConstructionCost(const FConstructionCost& Cost)
{
	if (!CanAffordConstruction(Cost))
	{
		UE_LOG(LogTemp, Warning, TEXT("ResourceManager: Cannot afford construction cost"));
		return false;
	}

	bool bSuccess = DeductResources(Cost.RequiredResources);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("ResourceManager: Paid construction cost (Work: %.0f, Workers: %d)"),
			Cost.RequiredWorkAmount, Cost.MaxWorkers);
		LogResourceStatus();
	}

	return bSuccess;
}

bool UResourceManagerSubsystem::RefundConstructionCost(const FConstructionCost& Cost, FVector Location)
{
	bool bSuccess = RefundResources(Cost.RequiredResources, Location);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("ResourceManager: Refunded construction cost at %s"), *Location.ToString());
		LogResourceStatus();
	}

	return bSuccess;
}

void UResourceManagerSubsystem::LogResourceStatus() const
{
	UE_LOG(LogTemp, Warning, TEXT("=== Resource Status ==="));

	if (CachedResourceTotals.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No resources available"));
		return;
	}

	// 자원 타입별로 정렬하여 출력
	TArray<EResourceType> ResourceTypes;
	CachedResourceTotals.GetKeys(ResourceTypes);

	for (EResourceType Type : ResourceTypes)
	{
		int32 Amount = CachedResourceTotals[Type];
		UE_LOG(LogTemp, Warning, TEXT("  %d: %d"), (int32)Type, Amount);
	}

	UE_LOG(LogTemp, Warning, TEXT("======================"));
}

void UResourceManagerSubsystem::LogResourceDetails(EResourceType ResourceType) const
{
	UE_LOG(LogTemp, Warning, TEXT("=== Resource Details: %d ==="), (int32)ResourceType);

	if (!GetWorld())
	{
		return;
	}

	UBuildingManagerSubsystem* BuildingManager = GetWorld()->GetSubsystem<UBuildingManagerSubsystem>();
	if (!BuildingManager)
	{
		return;
	}

	TArray<ABaseBuilding*> StorageBuildings = BuildingManager->GetAllStorageBuildings();

	int32 Total = 0;
	for (ABaseBuilding* Building : StorageBuildings)
	{
		if (Building && Building->Inventory)
		{
			int32 Amount = Building->Inventory->GetResourceQuantity(ResourceType);
			if (Amount > 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("  %s: %d"), *Building->BuildingName, Amount);
				Total += Amount;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Total: %d"), Total);
	UE_LOG(LogTemp, Warning, TEXT("======================"));
}
