// Copyright Epic Games, Inc. All Rights Reserved.

#include "BaseVillager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "VillagerAIController.h"
#include "TurnManagerSubsystem.h"
#include "InventoryComponent.h"
#include "House.h"
#include "TerrainZone.h"
#include "BaseBuilding.h"

// Sets default values
ABaseVillager::ABaseVillager()
{
 	// Set this character to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// Default values
	VillagerRole = EVillagerRole::Citizen;
	VillagerName = TEXT("Villager");
	WalkSpeed = 150.0f;
	RunSpeed = 300.0f;
	bIsPatrolling = true;
	CurrentPatrolIndex = 0;

	// State system defaults
	CurrentState = EActorState::IDLE;
	SocialClass = ESocialClass::Commoner;
	CurrentAction = EActionType::None;

	// Assignment system defaults
	AssignedHome = nullptr;
	AssignedWorkZone = nullptr;

	// Create inventory component
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	if (Inventory)
	{
		Inventory->MaxCapacity = 50; // Villagers can carry 50 items
	}

	// Create a simple cube mesh for visualization
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	if (BodyMesh)
	{
		// Find the cube mesh
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
		if (CubeMesh.Succeeded())
		{
			BodyMesh->SetStaticMesh(CubeMesh.Object);
			BodyMesh->SetupAttachment(RootComponent);
			BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
			BodyMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.0f));
		}
	}

	// Set default movement speed
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// Enable AI possession and set default AI controller
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Try to load BP_CitizenController, fallback to C++ class
	static ConstructorHelpers::FClassFinder<AVillagerAIController> AIControllerBP(TEXT("/Game/AI/BP_CitizenController"));
	if (AIControllerBP.Succeeded())
	{
		AIControllerClass = AIControllerBP.Class;
	}
	else
	{
		AIControllerClass = AVillagerAIController::StaticClass();
	}
}

// Called when the game starts or when spawned
void ABaseVillager::BeginPlay()
{
	Super::BeginPlay();

	SetMeshColor();
}

void ABaseVillager::SetMeshColor()
{
	if (!BodyMesh)
		return;

	// Create dynamic material instance
	UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BodyMesh->GetMaterial(0), this);

	if (DynMaterial)
	{
		FLinearColor Color;

		// Set color based on role
		switch (VillagerRole)
		{
		case EVillagerRole::Citizen:
			Color = FLinearColor::Blue;  // Blue for citizens
			break;
		case EVillagerRole::Guard:
			Color = FLinearColor::Red;   // Red for guards
			break;
		case EVillagerRole::Merchant:
			Color = FLinearColor::Yellow; // Yellow for merchants
			break;
		default:
			Color = FLinearColor::White;
			break;
		}

		DynMaterial->SetVectorParameterValue(FName("Color"), Color);
		BodyMesh->SetMaterial(0, DynMaterial);
	}
}

// Called every frame
void ABaseVillager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseVillager::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseVillager::RequestActionPermission(EActionType ActionType)
{
	UTurnManagerSubsystem* TurnManager = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
	if (!TurnManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No TurnManagerSubsystem available"), *GetName());
		return;
	}

	if (CurrentState != EActorState::IDLE)
	{
		UE_LOG(LogTemp, Verbose, TEXT("%s: Cannot request action, not IDLE (current state: %d)"), *GetName(), (int32)CurrentState);
		return;
	}

	TurnManager->RequestAction(this, ActionType, SocialClass);
	UE_LOG(LogTemp, Log, TEXT("%s requested action: %d"), *GetName(), (int32)ActionType);
}

void ABaseVillager::OnActionPermissionGranted(EActionType ActionType)
{
	CurrentAction = ActionType;

	// Change state based on action type
	switch (ActionType)
	{
	case EActionType::Move:
		CurrentState = EActorState::MOVING;
		break;
	case EActionType::Work:
		CurrentState = EActorState::WORKING;
		break;
	case EActionType::Fight:
		CurrentState = EActorState::FIGHTING;
		break;
	case EActionType::Trade:
		CurrentState = EActorState::TRADING;
		break;
	case EActionType::Rest:
		CurrentState = EActorState::RESTING;
		break;
	default:
		CurrentState = EActorState::IDLE;
		break;
	}

	UE_LOG(LogTemp, Warning, TEXT("%s: Action GRANTED - %d, State: %d"),
		*GetName(), (int32)ActionType, (int32)CurrentState);

	// TODO: Start the actual action (will be handled by BT)
}

void ABaseVillager::CompleteCurrentAction()
{
	UTurnManagerSubsystem* TurnManager = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
	if (!TurnManager)
		return;

	UE_LOG(LogTemp, Warning, TEXT("%s: Action COMPLETED - %d"), *GetName(), (int32)CurrentAction);

	// Notify turn manager
	TurnManager->NotifyActionComplete(this);

	// Reset to IDLE
	CurrentState = EActorState::IDLE;
	CurrentAction = EActionType::None;
}

bool ABaseVillager::AssignToHome(AHouse* Home)
{
	if (!Home)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Cannot assign to null home"), *VillagerName);
		return false;
	}

	// Unassign from previous home
	if (AssignedHome)
	{
		AssignedHome->RemoveResident(this);
	}

	// Assign to new home
	if (Home->AddResident(this))
	{
		AssignedHome = Home;
		UE_LOG(LogTemp, Log, TEXT("%s assigned to home '%s'"), *VillagerName, *Home->BuildingName);
		return true;
	}

	return false;
}

bool ABaseVillager::AssignToWorkZone(ATerrainZone* Zone)
{
	if (!Zone)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Cannot assign to null work zone"), *VillagerName);
		return false;
	}

	// Unassign from previous zone
	if (AssignedWorkZone)
	{
		UnassignFromWorkZone();
	}

	// Add to zone's worker list
	if (Zone->AddWorker(this))
	{
		AssignedWorkZone = Zone;
		UE_LOG(LogTemp, Log, TEXT("%s assigned to work zone '%s'"), *VillagerName, *Zone->ZoneName);
		return true;
	}

	return false;
}

void ABaseVillager::UnassignFromHome()
{
	if (AssignedHome)
	{
		AssignedHome->RemoveResident(this);
		UE_LOG(LogTemp, Log, TEXT("%s unassigned from home '%s'"), *VillagerName, *AssignedHome->BuildingName);
		AssignedHome = nullptr;
	}
}

void ABaseVillager::UnassignFromWorkZone()
{
	if (AssignedWorkZone)
	{
		AssignedWorkZone->RemoveWorker(this);
		UE_LOG(LogTemp, Log, TEXT("%s unassigned from work zone '%s'"), *VillagerName, *AssignedWorkZone->ZoneName);
		AssignedWorkZone = nullptr;
	}
}

// === Skill System ===

ESkillLevel ABaseVillager::GetSkillLevel(EBuildingType BuildingType) const
{
	// If villager has trained in this profession, return their skill level
	if (Skills.Contains(BuildingType))
	{
		return Skills[BuildingType];
	}

	// Default: Novice (can work Tier 1 buildings)
	return ESkillLevel::Novice;
}

void ABaseVillager::SetSkillLevel(EBuildingType BuildingType, ESkillLevel NewLevel)
{
	ESkillLevel OldLevel = GetSkillLevel(BuildingType);

	Skills.Add(BuildingType, NewLevel);

	UE_LOG(LogTemp, Log, TEXT("%s: Skill improved for %s - %d -> %d"),
		*VillagerName,
		*UEnum::GetValueAsString(BuildingType),
		(int32)OldLevel,
		(int32)NewLevel);
}

bool ABaseVillager::CanWorkAtBuilding(ABaseBuilding* Building) const
{
	if (!Building)
		return false;

	// Get villager's skill level for this building type
	ESkillLevel VillagerSkill = GetSkillLevel(Building->BuildingType);

	// Check if villager meets minimum requirement
	return VillagerSkill >= Building->RequiredSkillLevel;
}
