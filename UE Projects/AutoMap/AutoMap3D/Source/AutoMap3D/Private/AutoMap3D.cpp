// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoMap3D.h"
#include "GameplayTagsManager.h"

#define LOCTEXT_NAMESPACE "FAutoMap3DModule"

void FAutoMap3DModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UGameplayTagsManager::Get().AddTagIniSearchPath(FPaths::ProjectPluginsDir() / TEXT("AutoMap3D/Config/Tags"));
}

void FAutoMap3DModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAutoMap3DModule, AutoMap3D)