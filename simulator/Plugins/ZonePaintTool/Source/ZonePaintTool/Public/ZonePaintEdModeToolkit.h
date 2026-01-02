// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"
#include "SimulatorTypes.h"

class FZonePaintEdMode;

/**
 * Zone Paint Mode Toolkit
 * UI panel for Zone Paint settings
 */
class FZonePaintEdModeToolkit : public FModeToolkit
{
public:
	FZonePaintEdModeToolkit();

	// FModeToolkit interface
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost) override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual class FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<class SWidget> GetInlineContent() const override { return ToolkitWidget; }
	// End of FModeToolkit interface

	// Get current paint settings
	ETerrainZone GetCurrentZoneType() const { return CurrentZoneType; }
	int32 GetBrushSize() const { return BrushSize; }
	float GetResourceRichness() const { return ResourceRichness; }

	// Set current paint settings
	void SetCurrentZoneType(ETerrainZone ZoneType) { CurrentZoneType = ZoneType; }
	void SetBrushSize(int32 Size) { BrushSize = FMath::Clamp(Size, 1, 20); }
	void SetResourceRichness(float Richness) { ResourceRichness = FMath::Clamp(Richness, 0.0f, 1.0f); }

private:
	// Paint settings
	ETerrainZone CurrentZoneType = ETerrainZone::Farmland;
	int32 BrushSize = 3;
	float ResourceRichness = 1.0f;

	// UI Widget
	TSharedPtr<SWidget> ToolkitWidget;

	// Callbacks for UI
	FText GetCurrentZoneTypeText() const;
	FText GetBrushSizeText() const;
	FText GetResourceRichnessText() const;

	FReply OnZoneTypeButtonClicked(ETerrainZone ZoneType);
	FReply OnBrushSizeDecrement();
	FReply OnBrushSizeIncrement();
	void OnResourceRichnessChanged(float NewValue);
};
