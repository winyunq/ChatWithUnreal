// Copyright (c) 2025-2026 Winyunq. All rights reserved.
// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ChatWithUnreal : ModuleRules
{
	public ChatWithUnreal(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				ModuleDirectory + "/Public"
			}
		);
		
		PrivateIncludePaths.AddRange(
			new string[] {
				ModuleDirectory + "/Private"
			}
		);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"HTTP",
				"Json",
				"JsonUtilities",
				"Slate",
				"SlateCore",
				"UMG",
				"UnrealEd",
				"UmgMcp"
			}
		);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"EditorScriptingUtilities",
				"EditorSubsystem",
				"ApplicationCore",
				"Projects",
				"AssetRegistry",
				"WorkspaceMenuStructure",
				"ImageWrapper"
			}
		);
	}
}
