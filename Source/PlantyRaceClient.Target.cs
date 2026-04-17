// Copyright © 2026 33Fellowship. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class PlantyRaceClientTarget : TargetRules
{
    public PlantyRaceClientTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Client;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
        ExtraModuleNames.Add("PlantyRace");
    }
}