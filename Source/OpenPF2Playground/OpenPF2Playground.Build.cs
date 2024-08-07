// OpenPF2 for UE Game Logic, Copyright 2021, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

using UnrealBuildTool;

public class OpenPF2Playground : ModuleRules
{
	public OpenPF2Playground(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"HeadMountedDisplay",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"OpenPF2GameFramework",
			"EnhancedInput",
			"GameplayAbilities"
		});
	}
}
