// OpenPF2 for UE Game Logic, Copyright 2021-2026, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

using UnrealBuildTool;

public class OpenPF2PlaygroundTarget : TargetRules
{
	public OpenPF2PlaygroundTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.Add("OpenPF2Playground");

#if UE_5_9_OR_LATER || !UE_5_3_OR_LATER
		throw new BuildException("OpenPF2Playground supports Unreal Engine 5.3 through 5.8.");
#elif UE_5_8_OR_LATER
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
#elif UE_5_7_OR_LATER
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
#elif UE_5_6_OR_LATER
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
#elif UE_5_5_OR_LATER
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
#elif UE_5_4_OR_LATER
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
#else
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
#endif
	}
}
