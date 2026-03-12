#include "AutoMapMeshActorsEditorModule.h"
#include "Modules/ModuleManager.h"

#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Visualizers/AutoMapMeshTagVisualizer.h"
#include "AutoMapTagComponent.h"

IMPLEMENT_MODULE(FAutoMapMeshActorsEditor, AutoMapMeshActorsEditor);

void FAutoMapMeshActorsEditor::StartupModule()
{
    if (GUnrealEd)
    {
        TSharedPtr<FAutoMapMeshTagVisualizer> Visualizer = MakeShareable(new FAutoMapMeshTagVisualizer());

        GUnrealEd->RegisterComponentVisualizer(UAutoMapTagComponent::StaticClass()->GetFName(), Visualizer);

        Visualizer->OnRegister();
    }
}

void FAutoMapMeshActorsEditor::ShutdownModule()
{
    if (GUnrealEd)
    {
        GUnrealEd->UnregisterComponentVisualizer(UAutoMapTagComponent::StaticClass()->GetFName());
    }
}
