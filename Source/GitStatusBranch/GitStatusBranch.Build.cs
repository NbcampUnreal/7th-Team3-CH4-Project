using UnrealBuildTool;

public class GitStatusBranch: ModuleRules
{
    public GitStatusBranch(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine", "SourceControl"});
    }
}
