// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PlantyRace : ModuleRules
{
	public PlantyRace(ReadOnlyTargetRules Target) : base(Target)
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
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"AnimGraphRuntime",
			"NavigationSystem",
            "Niagara"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"PlantyRace",
			"PlantyRace/Variant_Platforming",
			"PlantyRace/Variant_Platforming/Animation",
			"PlantyRace/Variant_Combat",
			"PlantyRace/Variant_Combat/AI",
			"PlantyRace/Variant_Combat/Animation",
			"PlantyRace/Variant_Combat/Gameplay",
			"PlantyRace/Variant_Combat/Interfaces",
			"PlantyRace/Variant_Combat/UI",
			"PlantyRace/Variant_SideScrolling",
			"PlantyRace/Variant_SideScrolling/AI",
			"PlantyRace/Variant_SideScrolling/Gameplay",
			"PlantyRace/Variant_SideScrolling/Interfaces",
			"PlantyRace/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
