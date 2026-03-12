// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoMapMeshifyModule.h"
#include "AutoMapMeshifyEditorModeCommands.h"

#include "GameplayTagsManager.h"

#define LOCTEXT_NAMESPACE "AutoMapMeshifyModule"

void FAutoMapMeshifyModule::StartupModule()
{
	UGameplayTagsManager::Get().AddTagIniSearchPath(FPaths::ProjectPluginsDir() / TEXT("AutoMapMeshify/Config/Tags"));

	FAutoMapMeshifyEditorModeCommands::Register();
}

void FAutoMapMeshifyModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FAutoMapMeshifyEditorModeCommands::Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAutoMapMeshifyModule, AutoMapMeshify)