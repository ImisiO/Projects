using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class AutoMapMeshActorsEditor: ModuleRules
{
    public AutoMapMeshActorsEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "AutoMapMeshActors",
            }
            );

        PrivateDependencyModuleNames.AddRange(
            new string[] 
            {
                "Core",
                "CoreUObject",
                "Engine",
                "GameplayTags",
                "UnrealEd"
            }
            );
    }
}
