// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ZonePaintTool : ModuleRules
{
	public ZonePaintTool(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UnrealEd",
			"EditorFramework",
			"EditorSubsystem",
			"EditorInteractiveToolsFramework",
			"InteractiveToolsFramework",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"simulator" // Our main game module
		});
	}
}
