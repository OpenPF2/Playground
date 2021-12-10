// Copyright 2021 Guy Elsmore-Paddock. All Rights Reserved.
// Adapted from content that is Copyright Epic Games, Inc. (Third Person Sample).
// Licensed only for use with Unreal Engine.

using UnrealBuildTool;

public class OpenPF2Playground : ModuleRules
{
	public OpenPF2Playground(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
