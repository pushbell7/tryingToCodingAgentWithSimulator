// Copyright Epic Games, Inc. All Rights Reserved.

#include "VillagePlayerController.h"
#include "BaseVillager.h"
#include "VillagerAIController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AVillagePlayerController::AVillagePlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	CurrentPossessedVillager = nullptr;
	VillagerPreviousController = nullptr;
}

void AVillagePlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AVillagePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Bind mouse click
	InputComponent->BindAction("Click", IE_Pressed, this, &AVillagePlayerController::OnMouseClick);
}

void AVillagePlayerController::OnMouseClick()
{
	ABaseVillager* ClickedVillager = GetVillagerUnderCursor();

	if (ClickedVillager)
	{
		// Unpossess current villager first
		UnpossessVillager();

		// Store the villager's AI controller
		VillagerPreviousController = Cast<AAIController>(ClickedVillager->GetController());

		// Possess the new villager
		Possess(ClickedVillager);
		CurrentPossessedVillager = ClickedVillager;
	}
}

void AVillagePlayerController::PossessVillagerAtLocation(FVector Location)
{
	// Find closest villager to location
	TArray<AActor*> FoundVillagers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseVillager::StaticClass(), FoundVillagers);

	ABaseVillager* ClosestVillager = nullptr;
	float ClosestDistance = FLT_MAX;

	for (AActor* Actor : FoundVillagers)
	{
		ABaseVillager* Villager = Cast<ABaseVillager>(Actor);
		if (Villager)
		{
			float Distance = FVector::Dist(Villager->GetActorLocation(), Location);
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestVillager = Villager;
			}
		}
	}

	if (ClosestVillager)
	{
		UnpossessVillager();
		VillagerPreviousController = Cast<AAIController>(ClosestVillager->GetController());
		Possess(ClosestVillager);
		CurrentPossessedVillager = ClosestVillager;
	}
}

void AVillagePlayerController::PossessNearestVillager()
{
	APawn* CurrentPawn = GetPawn();
	if (CurrentPawn)
	{
		PossessVillagerAtLocation(CurrentPawn->GetActorLocation());
	}
}

void AVillagePlayerController::UnpossessVillager()
{
	if (CurrentPossessedVillager && VillagerPreviousController)
	{
		// Return control to AI
		VillagerPreviousController->Possess(CurrentPossessedVillager);
		CurrentPossessedVillager = nullptr;
		VillagerPreviousController = nullptr;
	}
}

ABaseVillager* AVillagePlayerController::GetVillagerUnderCursor()
{
	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Pawn, false, HitResult);

	if (HitResult.bBlockingHit)
	{
		return Cast<ABaseVillager>(HitResult.GetActor());
	}

	return nullptr;
}
