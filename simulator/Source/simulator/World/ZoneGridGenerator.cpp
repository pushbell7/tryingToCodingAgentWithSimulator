// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZoneGridGenerator.h"
#include "TerrainZone.h"
#include "ZoneManagerSubsystem.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AZoneGridGenerator::AZoneGridGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITOR
	// Make this actor visible in editor
	bIsEditorOnlyActor = true;
#endif
}

void AZoneGridGenerator::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
void AZoneGridGenerator::GenerateGridZones()
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("ZoneGridGenerator: No world!"));
		return;
	}

	// Clear existing zones if requested
	if (bClearExistingZones)
	{
		ClearZones();
	}

	UE_LOG(LogTemp, Log, TEXT("ZoneGridGenerator: Generating %dx%d grid (Cell Size: %.0f)"),
		GridSizeX, GridSizeY, CellSize);

	int32 ZonesCreated = 0;

	// Generate grid
	for (int32 Y = 0; Y < GridSizeY; Y++)
	{
		for (int32 X = 0; X < GridSizeX; X++)
		{
			// Get cell center position
			FVector CellCenter = GetCellCenter(X, Y);

			// Determine zone type
			ETerrainZone ZoneType = DefaultZoneType;

			if (bAutoDetectZoneType)
			{
				float Height = GetTerrainHeight(CellCenter);
				ZoneType = DetermineZoneType(CellCenter, Height);
			}

			// Spawn zone
			ATerrainZone* NewZone = SpawnZoneCell(X, Y, ZoneType);

			if (NewZone)
			{
				GeneratedZones.Add(NewZone);
				ZonesCreated++;
			}
		}
	}

	// Refresh ZoneManager to discover new zones
	if (bAutoRegisterZones)
	{
		UZoneManagerSubsystem* ZoneManager = GetWorld()->GetSubsystem<UZoneManagerSubsystem>();
		if (ZoneManager)
		{
			ZoneManager->RefreshZoneList();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ZoneGridGenerator: Created %d zones!"), ZonesCreated);

	// Show preview
	if (bShowDebugVisualization)
	{
		PreviewGrid();
	}
}

void AZoneGridGenerator::ClearGeneratedZones()
{
	ClearZones();
	UE_LOG(LogTemp, Log, TEXT("ZoneGridGenerator: Cleared all generated zones"));
}

void AZoneGridGenerator::PreviewGrid()
{
	if (!GetWorld())
		return;

	UE_LOG(LogTemp, Log, TEXT("ZoneGridGenerator: Drawing grid preview"));

	// Draw grid lines
	for (int32 Y = 0; Y <= GridSizeY; Y++)
	{
		FVector Start = GridOrigin + FVector(0, Y * CellSize, 100);
		FVector End = GridOrigin + FVector(GridSizeX * CellSize, Y * CellSize, 100);
		DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 10.0f, 0, 5.0f);
	}

	for (int32 X = 0; X <= GridSizeX; X++)
	{
		FVector Start = GridOrigin + FVector(X * CellSize, 0, 100);
		FVector End = GridOrigin + FVector(X * CellSize, GridSizeY * CellSize, 100);
		DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 10.0f, 0, 5.0f);
	}

	// Draw cell centers
	for (int32 Y = 0; Y < GridSizeY; Y++)
	{
		for (int32 X = 0; X < GridSizeX; X++)
		{
			FVector Center = GetCellCenter(X, Y);
			DrawDebugSphere(GetWorld(), Center, 50.0f, 8, FColor::Green, false, 10.0f, 0, 10.0f);
		}
	}
}
#endif

ETerrainZone AZoneGridGenerator::DetermineZoneType(FVector Location, float Height)
{
	// Water check
	if (Height < WaterHeightMax)
	{
		return ETerrainZone::Water;
	}

	// Mountain check
	if (Height > MountainHeightMin)
	{
		return ETerrainZone::Mountain;
	}

	// Random forest distribution on flat land
	float RandomValue = FMath::FRand();
	if (RandomValue < ForestProbability)
	{
		return ETerrainZone::Forest;
	}

	// Default to farmland for flat areas
	return ETerrainZone::Farmland;
}

FVector AZoneGridGenerator::GetCellCenter(int32 X, int32 Y) const
{
	float CenterX = GridOrigin.X + (X * CellSize) + (CellSize * 0.5f);
	float CenterY = GridOrigin.Y + (Y * CellSize) + (CellSize * 0.5f);
	float CenterZ = GridOrigin.Z;

	return FVector(CenterX, CenterY, CenterZ);
}

float AZoneGridGenerator::GetTerrainHeight(FVector Location) const
{
	if (!GetWorld())
		return 0.0f;

	// Raycast down to find terrain
	FHitResult Hit;
	FVector Start = Location + FVector(0, 0, 10000.0f); // Start high
	FVector End = Location - FVector(0, 0, 10000.0f);   // Trace down

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return Hit.Location.Z;
	}

	// No hit, return default
	return Location.Z;
}

ATerrainZone* AZoneGridGenerator::SpawnZoneCell(int32 GridX, int32 GridY, ETerrainZone ZoneType)
{
	if (!GetWorld())
		return nullptr;

	FVector CellCenter = GetCellCenter(GridX, GridY);

	// Adjust Z to terrain height if auto-detect is enabled
	if (bAutoDetectZoneType)
	{
		CellCenter.Z = GetTerrainHeight(CellCenter);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATerrainZone* NewZone = GetWorld()->SpawnActor<ATerrainZone>(
		ATerrainZone::StaticClass(),
		CellCenter,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (NewZone)
	{
		// Configure zone
		NewZone->ZoneType = ZoneType;
		NewZone->ZoneName = FString::Printf(TEXT("%s_Grid_%d_%d"),
			*UEnum::GetValueAsString(ZoneType),
			GridX,
			GridY);

		// Set folder for organization
#if WITH_EDITOR
		NewZone->SetFolderPath(FName(TEXT("GeneratedZones")));
#endif

		UE_LOG(LogTemp, Verbose, TEXT("Created zone: %s at (%d, %d)"),
			*NewZone->ZoneName, GridX, GridY);
	}

	return NewZone;
}

void AZoneGridGenerator::ClearZones()
{
	int32 ClearedCount = 0;

	for (ATerrainZone* Zone : GeneratedZones)
	{
		if (Zone && IsValid(Zone))
		{
			Zone->Destroy();
			ClearedCount++;
		}
	}

	GeneratedZones.Empty();

	UE_LOG(LogTemp, Log, TEXT("ZoneGridGenerator: Cleared %d zones"), ClearedCount);
}
