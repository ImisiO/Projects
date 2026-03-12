// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"

/**
 * This class contains info about the full set of commands used in this editor mode.
 */
class FAutoMapMeshifyEditorModeCommands : public TCommands<FAutoMapMeshifyEditorModeCommands>
{
public:
	FAutoMapMeshifyEditorModeCommands();

	virtual void RegisterCommands() override;
	static TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetCommands();

protected:
	TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> Commands;
};
