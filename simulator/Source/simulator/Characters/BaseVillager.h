// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

protected:
	// Current patrol index
	int32 CurrentPatrolIndex;

	// Set mesh color based on role
	void SetMeshColor();
};
