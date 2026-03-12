// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoMapMeshifyEditorMode.h"
#include "AutoMapMeshifyEditorModeToolkit.h"
#include "EdModeInteractiveToolsContext.h"
#include "InteractiveToolManager.h"
#include "AutoMapMeshifyEditorModeCommands.h"
#include "Modules/ModuleManager.h"


//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
// AddYourTool Step 1 - include the header file for your Tools here
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
#include "Tools/AutoMapMeshifySimpleTool.h"
#include "Tools/AutoMapMeshifyInteractiveTool.h"

// step 2: register a ToolBuilder in FAutoMapMeshifyEditorMode::Enter() below


#define LOCTEXT_NAMESPACE "AutoMapMeshifyEditorMode"

const FEditorModeID UAutoMapMeshifyEditorMode::EM_AutoMapMeshifyEditorModeId = TEXT("EM_AutoMapMeshifyEditorMode");

FString UAutoMapMeshifyEditorMode::MeshCoverterToolName = TEXT("AutoMapMeshify_MeshConverter");

UAutoMapMeshifyEditorMode::UAutoMapMeshifyEditorMode()
{
	FModuleManager::Get().LoadModule("EditorStyle");

	// appearance and icon in the editing mode ribbon can be customized here
	Info = FEditorModeInfo(UAutoMapMeshifyEditorMode::EM_AutoMapMeshifyEditorModeId,
		LOCTEXT("ModeName", "AutoMapMeshify"),
		FSlateIcon(),
		true);
}

UAutoMapMeshifyEditorMode::~UAutoMapMeshifyEditorMode()
{
}

void UAutoMapMeshifyEditorMode::ActorSelectionChangeNotify()
{
}

void UAutoMapMeshifyEditorMode::Enter()
{
	UEdMode::Enter();
	GetToolManager()->SelectActiveToolType(EToolSide::Left, MeshCoverterToolName);
}

void UAutoMapMeshifyEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FAutoMapMeshifyEditorModeToolkit);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UAutoMapMeshifyEditorMode::GetModeCommands() const
{
	return FAutoMapMeshifyEditorModeCommands::Get().GetCommands();
}

#undef LOCTEXT_NAMESPACE
