// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZonePaintEdModeToolkit.h"
#include "ZonePaintEdMode.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "FZonePaintEdModeToolkit"

FZonePaintEdModeToolkit::FZonePaintEdModeToolkit()
{
}

void FZonePaintEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	UE_LOG(LogTemp, Warning, TEXT("ZonePaintToolkit: Init called"));

	// Create UI widget
	ToolkitWidget =
		SNew(SVerticalBox)

		// Zone Type Selection
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ZoneTypeLabel", "Zone Type"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.0f)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(FMargin(2.0f))

			// Farmland
			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Farmland", "Farmland (1)"))
				.OnClicked(this, &FZonePaintEdModeToolkit::OnZoneTypeButtonClicked, ETerrainZone::Farmland)
			]

			// Pasture
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Pasture", "Pasture (2)"))
				.OnClicked(this, &FZonePaintEdModeToolkit::OnZoneTypeButtonClicked, ETerrainZone::Pasture)
			]

			// Forest
			+ SUniformGridPanel::Slot(0, 1)
			[
				SNew(SButton)
				.Text(LOCTEXT("Forest", "Forest (3)"))
				.OnClicked(this, &FZonePaintEdModeToolkit::OnZoneTypeButtonClicked, ETerrainZone::Forest)
			]

			// Mountain
			+ SUniformGridPanel::Slot(1, 1)
			[
				SNew(SButton)
				.Text(LOCTEXT("Mountain", "Mountain (4)"))
				.OnClicked(this, &FZonePaintEdModeToolkit::OnZoneTypeButtonClicked, ETerrainZone::Mountain)
			]

			// Water
			+ SUniformGridPanel::Slot(0, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("Water", "Water (5)"))
				.OnClicked(this, &FZonePaintEdModeToolkit::OnZoneTypeButtonClicked, ETerrainZone::Water)
			]

			// Settlement
			+ SUniformGridPanel::Slot(1, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("Settlement", "Settlement (6)"))
				.OnClicked(this, &FZonePaintEdModeToolkit::OnZoneTypeButtonClicked, ETerrainZone::Settlement)
			]
		]

		// Current Zone Type
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.0f, 10.0f, 2.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(this, &FZonePaintEdModeToolkit::GetCurrentZoneTypeText)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		// Brush Size
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.0f, 10.0f, 2.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BrushSizeLabel", "Brush Size"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("DecrementBrush", "-"))
				.OnClicked(this, &FZonePaintEdModeToolkit::OnBrushSizeDecrement)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(5.0f, 0.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(this, &FZonePaintEdModeToolkit::GetBrushSizeText)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("IncrementBrush", "+"))
				.OnClicked(this, &FZonePaintEdModeToolkit::OnBrushSizeIncrement)
			]
		]

		// Resource Richness
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.0f, 10.0f, 2.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ResourceRichnessLabel", "Resource Richness"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.0f)
		[
			SNew(SSlider)
			.Value(ResourceRichness)
			.OnValueChanged(this, &FZonePaintEdModeToolkit::OnResourceRichnessChanged)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(this, &FZonePaintEdModeToolkit::GetResourceRichnessText)
		];

	UE_LOG(LogTemp, Warning, TEXT("ZonePaintToolkit: Widget created, calling FModeToolkit::Init"));
	FModeToolkit::Init(InitToolkitHost);
	UE_LOG(LogTemp, Warning, TEXT("ZonePaintToolkit: FModeToolkit::Init completed"));
}

FName FZonePaintEdModeToolkit::GetToolkitFName() const
{
	return FName("ZonePaintMode");
}

FText FZonePaintEdModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("ToolkitName", "Zone Paint Mode");
}

FEdMode* FZonePaintEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FZonePaintEdMode::EM_ZonePaint);
}

FText FZonePaintEdModeToolkit::GetCurrentZoneTypeText() const
{
	FString TypeName = UEnum::GetDisplayValueAsText(CurrentZoneType).ToString();
	return FText::Format(LOCTEXT("CurrentZoneType", "Current: {0}"), FText::FromString(TypeName));
}

FText FZonePaintEdModeToolkit::GetBrushSizeText() const
{
	return FText::AsNumber(BrushSize);
}

FText FZonePaintEdModeToolkit::GetResourceRichnessText() const
{
	return FText::AsPercent(ResourceRichness);
}

FReply FZonePaintEdModeToolkit::OnZoneTypeButtonClicked(ETerrainZone ZoneType)
{
	CurrentZoneType = ZoneType;
	UE_LOG(LogTemp, Log, TEXT("ZonePaintToolkit: Selected %s"), *UEnum::GetValueAsString(ZoneType));
	return FReply::Handled();
}

FReply FZonePaintEdModeToolkit::OnBrushSizeDecrement()
{
	SetBrushSize(BrushSize - 1);
	return FReply::Handled();
}

FReply FZonePaintEdModeToolkit::OnBrushSizeIncrement()
{
	SetBrushSize(BrushSize + 1);
	return FReply::Handled();
}

void FZonePaintEdModeToolkit::OnResourceRichnessChanged(float NewValue)
{
	ResourceRichness = NewValue;
}

#undef LOCTEXT_NAMESPACE
