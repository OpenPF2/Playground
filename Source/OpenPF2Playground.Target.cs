// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OpenPF2PlaygroundTarget : TargetRules
{
	public OpenPF2PlaygroundTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("OpenPF2Playground");
	}
}
