// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ActionGame : ModuleRules
{
	public ActionGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", "MotionWarping" });

		PublicIncludePaths.Add("ActionGame/");

		PublicDependencyModuleNames.AddRange(new string[] {
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
		});
	}
}
