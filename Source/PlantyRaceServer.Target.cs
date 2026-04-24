// Copyright © 2026 33Fellowship. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class PlantyRaceServerTarget : TargetRules
{
    public PlantyRaceServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
        ExtraModuleNames.Add("PlantyRace");
    }
}