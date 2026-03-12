// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Toolkits/BaseToolkit.h"
#include "AutoMapMeshifyEditorMode.h"

class SAutoMapMeshifySidebarWidget;

class FAutoMapMeshifyEditorModeToolkit : public FModeToolkit
{
public:
	FAutoMapMeshifyEditorModeToolkit();

	/** FModeToolkit interface */
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode) override;
	virtual void GetToolPaletteNames(TArray<FName>& PaletteNames) const override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;

	virtual TSharedPtr<SWidget> GetInlineContent() const override;

private:
	TSharedPtr<SAutoMapMeshifySidebarWidget> SidebarWidget;
};
