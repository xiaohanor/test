// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class testcpp : ModuleRules
{
	public testcpp(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"HTTP",
			"Json",
			"JsonUtilities"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"testcpp",
			"testcpp/HTTP",
			"testcpp/LLM",
			"testcpp/AI",
			"testcpp/Variant_Platforming",
			"testcpp/Variant_Platforming/Animation",
			"testcpp/Variant_Combat",
			"testcpp/Variant_Combat/AI",
			"testcpp/Variant_Combat/Animation",
			"testcpp/Variant_Combat/Gameplay",
			"testcpp/Variant_Combat/Interfaces",
			"testcpp/Variant_Combat/UI",
			"testcpp/Variant_SideScrolling",
			"testcpp/Variant_SideScrolling/AI",
			"testcpp/Variant_SideScrolling/Gameplay",
			"testcpp/Variant_SideScrolling/Interfaces",
			"testcpp/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
