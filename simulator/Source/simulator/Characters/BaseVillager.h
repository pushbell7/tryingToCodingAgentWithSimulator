// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SimulatorTypes.h"
#include "BaseVillager.generated.h"

UENUM(BlueprintType)
enum class EVillagerRole : uint8
{
	Citizen		UMETA(DisplayName = "Citizen"),
	Guard		UMETA(DisplayName = "Guard"),
	Merchant	UMETA(DisplayName = "Merchant")
};

UCLASS()
class SIMULATOR_API ABaseVillager : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseVillager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Villager properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Villager")
	EVillagerRole VillagerRole;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Villager")
	FString VillagerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Villager")
	float WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Villager")
	float RunSpeed;

	// AI Behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	bool bIsPatrolling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<FVector> PatrolPoints;

	// Visual components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	class UStaticMeshComponent* BodyMesh;

	// Inventory system
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	class UInventoryComponent* Inventory;

	// State & Action System
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	EActorState CurrentState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	ESocialClass SocialClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	EActionType CurrentAction;

	// === Skill System (Guild-based) ===

	// Skill levels for different building types (what this villager can work on)
	// Maps building type -> skill level for that profession
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
	TMap<EBuildingType, ESkillLevel> Skills;

	// Get skill level for a specific building type
	UFUNCTION(BlueprintCallable, Category = "Skills")
	ESkillLevel GetSkillLevel(EBuildingType BuildingType) const;

	// Set/improve skill level for a building type
	UFUNCTION(BlueprintCallable, Category = "Skills")
	void SetSkillLevel(EBuildingType BuildingType, ESkillLevel NewLevel);

	// Check if villager can work at a building (meets minimum skill requirement)
	UFUNCTION(BlueprintCallable, Category = "Skills")
	bool CanWorkAtBuilding(class ABaseBuilding* Building) const;

	// Assignment System
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assignment")
	class AHouse* AssignedHome;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assignment")
	class ATerrainZone* AssignedWorkZone;

	// Assign villager to a home
	UFUNCTION(BlueprintCallable, Category = "Assignment")
	bool AssignToHome(class AHouse* Home);

	// Assign villager to a work zone
	UFUNCTION(BlueprintCallable, Category = "Assignment")
	bool AssignToWorkZone(class ATerrainZone* Zone);

	// Unassign from home
	UFUNCTION(BlueprintCallable, Category = "Assignment")
	void UnassignFromHome();

	// Unassign from work zone
	UFUNCTION(BlueprintCallable, Category = "Assignment")
	void UnassignFromWorkZone();

	// Request action from turn manager
	UFUNCTION(BlueprintCallable, Category = "Turn System")
	void RequestActionPermission(EActionType ActionType);

	// Called when action permission is granted
	UFUNCTION(BlueprintCallable, Category = "Turn System")
	void OnActionPermissionGranted(EActionType ActionType);

	// Notify turn manager that action is complete
	UFUNCTION(BlueprintCallable, Category = "Turn System")
	void CompleteCurrentAction();

protected:
	// Current patrol index
	int32 CurrentPatrolIndex;

	// Set mesh color based on role
	void SetMeshColor();
};
