// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZonePaintToolModule.h"
#include "ZonePaintEdMode.h"
#include "EditorModeRegistry.h"

#define LOCTEXT_NAMESPACE "FZonePaintToolModule"

void FZonePaintToolModule::StartupModule()
{
	// Register the editor mode
	FEditorModeRegistry::Get().RegisterMode<FZonePaintEdMode>(
		FZonePaintEdMode::EM_ZonePaint,
		LOCTEXT("ZonePaintModeName", "Zone Paint"),
		FSlateIcon(),
		true
	);
}

void FZonePaintToolModule::ShutdownModule()
{
	// Unregister the editor mode
	FEditorModeRegistry::Get().UnregisterMode(FZonePaintEdMode::EM_ZonePaint);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FZonePaintToolModule, ZonePaintTool)
