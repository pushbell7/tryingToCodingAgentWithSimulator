// Copyright Epic Games, Inc. All Rights Reserved.

#include "BaseVillager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "VillagerAIController.h"

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
