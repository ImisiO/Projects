// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoMapMeshifyEditorModeCommands.h"
#include "AutoMapMeshifyEditorMode.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "AutoMapMeshifyEditorModeCommands"

FAutoMapMeshifyEditorModeCommands::FAutoMapMeshifyEditorModeCommands()
	: TCommands<FAutoMapMeshifyEditorModeCommands>("AutoMapMeshifyEditorMode",
		NSLOCTEXT("AutoMapMeshifyEditorMode", "AutoMapMeshifyEditorModeCommands", "AutoMapMeshify Editor Mode"),
		NAME_None,
		FAppStyle::GetAppStyleSetName())
{
}

void FAutoMapMeshifyEditorModeCommands::RegisterCommands()
{
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FAutoMapMeshifyEditorModeCommands::GetCommands()
{
	return FAutoMapMeshifyEditorModeCommands::Get().Commands;
}

#undef LOCTEXT_NAMESPACE
