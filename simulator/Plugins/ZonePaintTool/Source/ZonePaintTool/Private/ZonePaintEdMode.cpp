// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZonePaintEdMode.h"
#include "ZonePaintEdModeToolkit.h"
#include "ZoneGrid.h"
#include "EditorModeManager.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Toolkits/ToolkitManager.h"

const FEditorModeID FZonePaintEdMode::EM_ZonePaint(TEXT("EM_ZonePaint"));

FZonePaintEdMode::FZonePaintEdMode()
{
}

FZonePaintEdMode::~FZonePaintEdMode()
{
}

void FZonePaintEdMode::Enter()
{
	FEdMode::Enter();

	UE_LOG(LogTemp, Warning, TEXT("========================================"));
	UE_LOG(LogTemp, Warning, TEXT("ZonePaintEdMode: ENTERED Zone Paint Mode"));
	UE_LOG(LogTemp, Warning, TEXT("========================================"));

	// Manually create toolkit if not already created
	if (!Toolkit.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("ZonePaintEdMode: Toolkit not valid, creating..."));
		CreateToolkit();
	}
}

void FZonePaintEdMode::Exit()
{
	bIsPainting = false;

	UE_LOG(LogTemp, Log, TEXT("ZonePaintEdMode: Exited Zone Paint Mode"));

	FEdMode::Exit();
}

void FZonePaintEdMode::CreateToolkit()
{
	UE_LOG(LogTemp, Warning, TEXT("ZonePaintEdMode: CreateToolkit called"));

	Toolkit = MakeShareable(new FZonePaintEdModeToolkit);

	if (Owner && Owner->GetToolkitHost())
	{
		UE_LOG(LogTemp, Warning, TEXT("ZonePaintEdMode: Initializing Toolkit"));
		Toolkit->Init(Owner->GetToolkitHost());
		UE_LOG(LogTemp, Warning, TEXT("ZonePaintEdMode: Toolkit initialized successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ZonePaintEdMode: Owner or ToolkitHost is null!"));
	}
}

void FZonePaintEdMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	FEdMode::Tick(ViewportClient, DeltaTime);

	// Update cursor location for brush preview
	if (ViewportClient && ViewportClient->Viewport)
	{
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			ViewportClient->Viewport,
			ViewportClient->GetScene(),
			ViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(ViewportClient->IsRealtime()));

		FSceneView* View = ViewportClient->CalcSceneView(&ViewFamily);

		if (View)
		{
			FViewportCursorLocation Cursor(View, ViewportClient, ViewportClient->Viewport->GetMouseX(), ViewportClient->Viewport->GetMouseY());
			FVector CursorOrigin = Cursor.GetOrigin();
			FVector CursorDirection = Cursor.GetDirection();

			// Trace to find world location
			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.bTraceComplex = false;

			if (GetWorld()->LineTraceSingleByChannel(Hit, CursorOrigin, CursorOrigin + CursorDirection * 100000.0f, ECC_Visibility, Params))
			{
				CursorWorldLocation = Hit.Location;
				bCursorValid = true;

				// If painting, paint at this location
				if (bIsPainting)
				{
					PaintAtLocation(CursorWorldLocation);
				}
			}
			else
			{
				bCursorValid = false;
			}
		}
	}
}

void FZonePaintEdMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	FEdMode::Render(View, Viewport, PDI);

	// Draw brush preview at cursor location
	if (bCursorValid)
	{
		AZoneGrid* ZoneGrid = GetCurrentZoneGrid();
		FZonePaintEdModeToolkit* PaintToolkit = static_cast<FZonePaintEdModeToolkit*>(Toolkit.Get());

		if (ZoneGrid && PaintToolkit)
		{
			float CellSize = ZoneGrid->CellSize;
			int32 CurrentBrushSize = PaintToolkit->GetBrushSize(); // Use Toolkit's brush size
			FVector BrushCenter = CursorWorldLocation;
			BrushCenter.Z += 50.0f; // Offset above terrain

			// Draw brush circle
			float BrushRadius = CurrentBrushSize * CellSize;
			DrawCircle(PDI, BrushCenter, FVector(1, 0, 0), FVector(0, 1, 0), FColor::Yellow, BrushRadius, 32, SDPG_Foreground, 2.0f);

			// Draw center cross
			FVector CrossSize(CellSize * 0.5f, CellSize * 0.5f, 0);
			PDI->DrawLine(BrushCenter - FVector(CrossSize.X, 0, 0), BrushCenter + FVector(CrossSize.X, 0, 0), FColor::Yellow, SDPG_Foreground, 2.0f);
			PDI->DrawLine(BrushCenter - FVector(0, CrossSize.Y, 0), BrushCenter + FVector(0, CrossSize.Y, 0), FColor::Yellow, SDPG_Foreground, 2.0f);
		}
	}
}

bool FZonePaintEdMode::HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
	UE_LOG(LogTemp, Warning, TEXT("ZonePaint: HandleClick called - Key: %s, Event: %d"), *Click.GetKey().ToString(), (int32)Click.GetEvent());

	if (Click.GetKey() == EKeys::LeftMouseButton)
	{
		if (Click.GetEvent() == IE_Pressed)
		{
			UE_LOG(LogTemp, Warning, TEXT("ZonePaint: Left mouse pressed, bCursorValid: %d"), bCursorValid);
			bIsPainting = true;
			if (bCursorValid)
			{
				UE_LOG(LogTemp, Warning, TEXT("ZonePaint: Painting at location: %s"), *CursorWorldLocation.ToString());
				PaintAtLocation(CursorWorldLocation);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ZonePaint: Cursor not valid!"));
			}
			return true; // Consume the click
		}
		else if (Click.GetEvent() == IE_Released)
		{
			UE_LOG(LogTemp, Warning, TEXT("ZonePaint: Left mouse released"));
			bIsPainting = false;
			return true; // Consume the release
		}
	}

	// Don't allow any clicks through to default handler (prevents selection)
	return true;
}

bool FZonePaintEdMode::InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale)
{
	// Allow camera movement
	return FEdMode::InputDelta(InViewportClient, InViewport, InDrag, InRot, InScale);
}

bool FZonePaintEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	// Handle left mouse button for painting
	if (Key == EKeys::LeftMouseButton)
	{
		if (Event == IE_Pressed)
		{
			UE_LOG(LogTemp, Warning, TEXT("ZonePaint: InputKey - Left mouse PRESSED, bCursorValid: %d"), bCursorValid);
			bIsPainting = true;
			if (bCursorValid)
			{
				UE_LOG(LogTemp, Warning, TEXT("ZonePaint: Painting from InputKey at location: %s"), *CursorWorldLocation.ToString());
				PaintAtLocation(CursorWorldLocation);
			}
			return true;
		}
		else if (Event == IE_Released)
		{
			UE_LOG(LogTemp, Warning, TEXT("ZonePaint: InputKey - Left mouse RELEASED"));
			bIsPainting = false;
			return true;
		}
	}

	// Handle brush size adjustment and zone type selection
	FZonePaintEdModeToolkit* PaintToolkit = static_cast<FZonePaintEdModeToolkit*>(Toolkit.Get());
	if (PaintToolkit && Event == IE_Pressed)
	{
		if (Key == EKeys::LeftBracket)
		{
			PaintToolkit->SetBrushSize(PaintToolkit->GetBrushSize() - 1);
			UE_LOG(LogTemp, Warning, TEXT("ZonePaint: Brush size decreased to %d"), PaintToolkit->GetBrushSize());
			return true;
		}
		else if (Key == EKeys::RightBracket)
		{
			PaintToolkit->SetBrushSize(PaintToolkit->GetBrushSize() + 1);
			UE_LOG(LogTemp, Warning, TEXT("ZonePaint: Brush size increased to %d"), PaintToolkit->GetBrushSize());
			return true;
		}
		// Number keys 1-6 for zone type selection
		else if (Key == EKeys::One)
		{
			PaintToolkit->SetCurrentZoneType(ETerrainZone::Farmland);
			UE_LOG(LogTemp, Warning, TEXT("ZonePaint: Selected Farmland"));
			return true;
		}
		else if (Key == EKeys::Two)
		{
			PaintToolkit->SetCurrentZoneType(ETerrainZone::Pasture);
			UE_LOG(LogTemp, Warning, TEXT("ZonePaint: Selected Pasture"));
			return true;
		}
		else if (Key == EKeys::Three)
		{
			PaintToolkit->SetCurrentZoneType(ETerrainZone::Forest);
			UE_LOG(LogTemp, Warning, TEXT("ZonePaint: Selected Forest"));
			return true;
		}
		else if (Key == EKeys::Four)
		{
			PaintToolkit->SetCurrentZoneType(ETerrainZone::Mountain);
			UE_LOG(LogTemp, Warning, TEXT("ZonePaint: Selected Mountain"));
			return true;
		}
		else if (Key == EKeys::Five)
		{
			PaintToolkit->SetCurrentZoneType(ETerrainZone::Water);
			UE_LOG(LogTemp, Warning, TEXT("ZonePaint: Selected Water"));
			return true;
		}
		else if (Key == EKeys::Six)
		{
			PaintToolkit->SetCurrentZoneType(ETerrainZone::Settlement);
			UE_LOG(LogTemp, Warning, TEXT("ZonePaint: Selected Settlement"));
			return true;
		}
	}

	return FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
}

AZoneGrid* FZonePaintEdMode::GetCurrentZoneGrid() const
{
	UWorld* World = GetWorld();
	if (!World)
		return nullptr;

	// Find first ZoneGrid in the level
	for (TActorIterator<AZoneGrid> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

void FZonePaintEdMode::PaintAtLocation(const FVector& WorldLocation)
{
	AZoneGrid* ZoneGrid = GetCurrentZoneGrid();
	FZonePaintEdModeToolkit* PaintToolkit = static_cast<FZonePaintEdModeToolkit*>(Toolkit.Get());

	if (!ZoneGrid)
	{
		UE_LOG(LogTemp, Warning, TEXT("ZonePaint: No ZoneGrid found in level!"));
		return;
	}

	if (!PaintToolkit)
	{
		UE_LOG(LogTemp, Warning, TEXT("ZonePaint: Toolkit not initialized!"));
		return;
	}

	// Use Toolkit's settings
	ETerrainZone ZoneType = PaintToolkit->GetCurrentZoneType();
	int32 BrushSize = PaintToolkit->GetBrushSize();

	ZoneGrid->PaintZoneArea(WorldLocation, BrushSize, ZoneType);
}
