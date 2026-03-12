using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class AutoMapMeshActors: ModuleRules
{
    public AutoMapMeshActors(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine", "GameplayTags" });
    }
}
