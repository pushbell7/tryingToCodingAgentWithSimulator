// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZoneGrid.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

AZoneGrid::AZoneGrid()
{
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITOR
	bIsEditorOnlyActor = false; // We want this in runtime too
#endif

	// Set default grid origin to actor location
	GridOrigin = GetActorLocation();
}

void AZoneGrid::BeginPlay()
{
	Super::BeginPlay();

	// Ensure grid is initialized at runtime
	if (ZoneCells.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ZoneGrid: No cell data found at runtime. Initialize grid in editor first!"));
	}
}

#if WITH_EDITOR
void AZoneGrid::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Re-visualize when visualization settings change
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AZoneGrid, bShowGridVisualization) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(AZoneGrid, bShowCellBorders))
	{
		if (bShowGridVisualization)
		{
			VisualizeGrid();
		}
	}

	// Update grid origin when actor moves
	if (PropertyName == FName("RelativeLocation"))
	{
		GridOrigin = GetActorLocation();
	}
}

void AZoneGrid::InitializeGrid()
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("ZoneGrid: No world!"));
		return;
	}

	int32 TotalCells = GridSizeX * GridSizeY;
	UE_LOG(LogTemp, Log, TEXT("ZoneGrid: Initializing %dx%d grid (%d cells, Cell Size: %.0f)"),
		GridSizeX, GridSizeY, TotalCells, CellSize);

	// Clear existing data
	ZoneCells.Empty();
	ZoneCells.Reserve(TotalCells);

	// Create all cells
	for (int32 Y = 0; Y < GridSizeY; Y++)
	{
		for (int32 X = 0; X < GridSizeX; X++)
		{
			FVector WorldPos = GridCoordsToWorld(X, Y);
			ETerrainZone ZoneType = DefaultZoneType;

			// Auto-detect zone type if enabled
			if (bAutoDetectZoneType)
			{
				float Height = GetTerrainHeight(WorldPos);
				ZoneType = DetermineZoneType(WorldPos, Height);
			}

			// Create cell
			FZoneCellData NewCell(ZoneType, FIntPoint(X, Y), WorldPos);
			ZoneCells.Add(NewCell);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ZoneGrid: Created %d cells!"), ZoneCells.Num());

	// Visualize
	if (bShowGridVisualization)
	{
		VisualizeGrid();
	}
}

void AZoneGrid::ClearGrid()
{
	ZoneCells.Empty();
	UE_LOG(LogTemp, Log, TEXT("ZoneGrid: Cleared all cells"));

	// Clear visualization
	if (GetWorld())
	{
		FlushPersistentDebugLines(GetWorld());
	}
}

void AZoneGrid::AutoGenerateZoneTypes()
{
	if (ZoneCells.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ZoneGrid: No cells to generate! Initialize grid first."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ZoneGrid: Auto-generating zone types for %d cells..."), ZoneCells.Num());

	int32 UpdatedCount = 0;
	for (FZoneCellData& Cell : ZoneCells)
	{
		float Height = GetTerrainHeight(Cell.WorldPosition);
		ETerrainZone NewType = DetermineZoneType(Cell.WorldPosition, Height);

		if (Cell.ZoneType != NewType)
		{
			Cell.ZoneType = NewType;
			UpdatedCount++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ZoneGrid: Updated %d cells"), UpdatedCount);

	// Re-visualize
	if (bShowGridVisualization)
	{
		VisualizeGrid();
	}
}

void AZoneGrid::VisualizeGrid()
{
	if (!GetWorld())
		return;

	// Clear previous visualization
	FlushPersistentDebugLines(GetWorld());

	if (!bShowGridVisualization)
		return;

	UE_LOG(LogTemp, Log, TEXT("ZoneGrid: Visualizing grid..."));

	// Draw grid borders
	if (bShowCellBorders)
	{
		// Horizontal lines
		for (int32 Y = 0; Y <= GridSizeY; Y++)
		{
			FVector Start = GridOrigin + FVector(0, Y * CellSize, VisualizationHeight);
			FVector End = GridOrigin + FVector(GridSizeX * CellSize, Y * CellSize, VisualizationHeight);
			DrawDebugLine(GetWorld(), Start, End, FColor::White, true, -1.0f, 0, 10.0f);
		}

		// Vertical lines
		for (int32 X = 0; X <= GridSizeX; X++)
		{
			FVector Start = GridOrigin + FVector(X * CellSize, 0, VisualizationHeight);
			FVector End = GridOrigin + FVector(X * CellSize, GridSizeY * CellSize, VisualizationHeight);
			DrawDebugLine(GetWorld(), Start, End, FColor::White, true, -1.0f, 0, 10.0f);
		}
	}

	// Draw cell colors
	for (const FZoneCellData& Cell : ZoneCells)
	{
		FColor CellColor = GetZoneColor(Cell.ZoneType);

		// Draw filled square for cell
		FVector CellMin = GridOrigin + FVector(
			Cell.GridCoords.X * CellSize,
			Cell.GridCoords.Y * CellSize,
			VisualizationHeight
		);
		FVector CellMax = CellMin + FVector(CellSize, CellSize, 0);

		// Draw box
		FVector Center = (CellMin + CellMax) * 0.5f;
		FVector Extent = FVector(CellSize * 0.45f, CellSize * 0.45f, 5.0f); // Slightly smaller to show borders
		DrawDebugBox(GetWorld(), Center, Extent, CellColor, true, -1.0f, 0, 5.0f);
	}

	UE_LOG(LogTemp, Log, TEXT("ZoneGrid: Visualization complete"));
}
#endif

void AZoneGrid::PaintZoneArea(FVector WorldLocation, int32 BrushRadius, ETerrainZone ZoneType)
{
	if (ZoneCells.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ZoneGrid: No cells to paint! Initialize grid first."));
		return;
	}

	FIntPoint CenterCoords = WorldToGridCoords(WorldLocation);

	if (!IsValidGridCoords(CenterCoords.X, CenterCoords.Y))
	{
		UE_LOG(LogTemp, Warning, TEXT("ZoneGrid: Paint location outside grid bounds"));
		return;
	}

	int32 PaintedCount = 0;

	// Paint all cells within brush radius
	for (int32 OffsetY = -BrushRadius; OffsetY <= BrushRadius; OffsetY++)
	{
		for (int32 OffsetX = -BrushRadius; OffsetX <= BrushRadius; OffsetX++)
		{
			// Check if within circular brush
			float Distance = FMath::Sqrt(static_cast<float>(OffsetX * OffsetX + OffsetY * OffsetY));
			if (Distance > BrushRadius)
				continue;

			int32 X = CenterCoords.X + OffsetX;
			int32 Y = CenterCoords.Y + OffsetY;

			if (!IsValidGridCoords(X, Y))
				continue;

			int32 Index = GridCoordsToIndex(X, Y);
			if (ZoneCells.IsValidIndex(Index))
			{
				ZoneCells[Index].ZoneType = ZoneType;
				// ResourceRichness is managed by Editor Mode Toolkit
				PaintedCount++;
			}
		}
	}

	if (PaintedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("ZoneGrid: Painted %d cells with %s"),
			PaintedCount, *UEnum::GetValueAsString(ZoneType));

		// Re-visualize
		if (bShowGridVisualization)
		{
			VisualizeGrid();
		}
	}
}

// === Query Functions ===

ETerrainZone AZoneGrid::GetZoneTypeAtLocation(FVector WorldLocation) const
{
	FZoneCellData Cell;
	if (GetCellAtLocation(WorldLocation, Cell))
	{
		return Cell.ZoneType;
	}
	return ETerrainZone::Farmland; // Default
}

ETerrainZone AZoneGrid::GetZoneTypeAtGridCoords(int32 X, int32 Y) const
{
	FZoneCellData Cell;
	if (GetCellAtGridCoords(X, Y, Cell))
	{
		return Cell.ZoneType;
	}
	return ETerrainZone::Farmland; // Default
}

bool AZoneGrid::GetCellAtLocation(FVector WorldLocation, FZoneCellData& OutCell) const
{
	FIntPoint Coords = WorldToGridCoords(WorldLocation);
	return GetCellAtGridCoords(Coords.X, Coords.Y, OutCell);
}

bool AZoneGrid::GetCellAtGridCoords(int32 X, int32 Y, FZoneCellData& OutCell) const
{
	if (!IsValidGridCoords(X, Y))
		return false;

	int32 Index = GridCoordsToIndex(X, Y);
	if (!ZoneCells.IsValidIndex(Index))
		return false;

	OutCell = ZoneCells[Index];
	return true;
}

bool AZoneGrid::CanBuildAtLocation(FVector WorldLocation, ETerrainZone RequiredZoneType) const
{
	ETerrainZone ZoneType = GetZoneTypeAtLocation(WorldLocation);
	return ZoneType == RequiredZoneType;
}

FIntPoint AZoneGrid::WorldToGridCoords(FVector WorldLocation) const
{
	FVector LocalPos = WorldLocation - GridOrigin;
	int32 X = FMath::FloorToInt(LocalPos.X / CellSize);
	int32 Y = FMath::FloorToInt(LocalPos.Y / CellSize);
	return FIntPoint(X, Y);
}

FVector AZoneGrid::GridCoordsToWorld(int32 X, int32 Y) const
{
	// Return center of cell
	float WorldX = GridOrigin.X + (X * CellSize) + (CellSize * 0.5f);
	float WorldY = GridOrigin.Y + (Y * CellSize) + (CellSize * 0.5f);
	float WorldZ = GridOrigin.Z;
	return FVector(WorldX, WorldY, WorldZ);
}

// === Helper Functions ===

int32 AZoneGrid::GridCoordsToIndex(int32 X, int32 Y) const
{
	return Y * GridSizeX + X;
}

bool AZoneGrid::IsValidGridCoords(int32 X, int32 Y) const
{
	return X >= 0 && X < GridSizeX && Y >= 0 && Y < GridSizeY;
}

float AZoneGrid::GetTerrainHeight(FVector Location) const
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

ETerrainZone AZoneGrid::DetermineZoneType(FVector Location, float Height) const
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

FColor AZoneGrid::GetZoneColor(ETerrainZone ZoneType) const
{
	switch (ZoneType)
	{
	case ETerrainZone::Farmland:
		return FColor(255, 235, 120); // Light yellow-green
	case ETerrainZone::Forest:
		return FColor(34, 139, 34); // Forest green
	case ETerrainZone::Mountain:
		return FColor(139, 137, 137); // Gray
	case ETerrainZone::Water:
		return FColor(65, 105, 225); // Royal blue
	case ETerrainZone::Settlement:
		return FColor(192, 192, 192); // Silver
	case ETerrainZone::Pasture:
		return FColor(144, 238, 144); // Light green
	default:
		return FColor::White;
	}
}
