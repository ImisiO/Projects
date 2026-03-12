// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoMapMeshifyEditorModeToolkit.h"
#include "AutoMapMeshifyEditorMode.h"
#include "Engine/Selection.h"
#include "Widgets/AutoMapMeshifySidebarWidget.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "AutoMapMeshifyEditorModeToolkit"

FAutoMapMeshifyEditorModeToolkit::FAutoMapMeshifyEditorModeToolkit()
{
}

void FAutoMapMeshifyEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	SidebarWidget = SNew(SAutoMapMeshifySidebarWidget);

	FModeToolkit::Init(InitToolkitHost, InOwningMode);
}

void FAutoMapMeshifyEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
}

FName FAutoMapMeshifyEditorModeToolkit::GetToolkitFName() const
{
	return FName("AutoMapMeshifyEditorMode");
}

FText FAutoMapMeshifyEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DisplayName", "AutoMapMeshifyEditorMode Toolkit");
}

TSharedPtr<SWidget> FAutoMapMeshifyEditorModeToolkit::GetInlineContent() const
{
	return SidebarWidget;
}

#undef LOCTEXT_NAMESPACE
