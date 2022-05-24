// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;

public class NePythonConsole : ModuleRules
{
	public NePythonConsole(ReadOnlyTargetRules Target) : base(Target)
	{
		bUsePrecompiled = true;
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"UnrealEd",
				"Slate",
				"SlateCore",
				"EditorStyle",
				"TargetPlatform",
				"NePythonBinding"
			}
		);
	}
}
