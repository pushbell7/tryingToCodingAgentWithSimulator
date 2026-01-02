// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"
#include "SimulatorTypes.h"

class AZoneGrid;

/**
 * Zone Paint Editor Mode
 * Allows painting zone types on ZoneGrid with mouse click/drag
 */
class FZonePaintEdMode : public FEdMode
{
public:
	const static FEditorModeID EM_ZonePaint;

	FZonePaintEdMode();
	virtual ~FZonePaintEdMode();

	// FEdMode interface
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	virtual bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click) override;
	virtual bool InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale) override;
	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	virtual bool UsesToolkits() const override { return true; }
	void CreateToolkit();
	virtual bool AllowWidgetMove() override { return false; }
	virtual bool ShouldDrawWidget() const override { return false; }
	virtual bool UsesTransformWidget() const override { return false; }
	virtual bool DisallowMouseDeltaTracking() const override { return bIsPainting; }
	// End of FEdMode interface

	// Get current zone grid in the level
	AZoneGrid* GetCurrentZoneGrid() const;

protected:
	// Paint at world location
	void PaintAtLocation(const FVector& WorldLocation);

	// Current mouse cursor location
	FVector CursorWorldLocation;
	bool bCursorValid = false;

	// Is mouse button held down for painting?
	bool bIsPainting = false;
};
