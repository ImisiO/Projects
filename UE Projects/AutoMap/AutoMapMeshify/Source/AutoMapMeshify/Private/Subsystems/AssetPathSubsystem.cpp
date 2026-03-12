#include "Subsystems/AssetPathSubsystem.h"
#include "EditorAssetLibrary.h"
#include "Misc/Paths.h"
#include "Logging/StructuredLog.h"
#include "HAL/FileManager.h"

DEFINE_LOG_CATEGORY(AutoMapMeshifyPathSubsystem);

// Public Functions --------------------------------------------------------------------------------------------

void UAssetPathSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(AutoMapMeshifyPathSubsystem, Display, TEXT("Initializing AutoMap Meshify Asset Path Editor Subsystem"));

	SetupPathsAndFolders();
}

void UAssetPathSubsystem::Deinitialize()
{
	UE_LOG(AutoMapMeshifyPathSubsystem, Display, TEXT("Deinitializing AutoMap Meshify Asset Path Editor Subsystem"));

	Super::Deinitialize();
}

// Platform asbolute path functions

const FString& UAssetPathSubsystem::GetProjectContentAbsPath() const
{
	return ProjectContentPlatformAbsPath;
}

FString UAssetPathSubsystem::GetUEGeneratedTerrainMeshesAbsPath() const
{
	TArray<FStringFormatArg> Args;
	Args.Add(FStringFormatArg(ProjectContentPlatformAbsPath));
	Args.Add(FStringFormatArg(PathConstants::MapMeshesDirName));
	Args.Add(FStringFormatArg(TEXT("/")));
	Args.Add(FStringFormatArg(PathConstants::ExportUEDirName));
	Args.Add(FStringFormatArg(TEXT("/")));
	Args.Add(FStringFormatArg(PathConstants::TerrainMeshesDirName));
	Args.Add(FStringFormatArg(TEXT("/")));

	FString Path = FString::Format(TEXT("{0}{1}{2}{3}{4}{5}{6}"), Args);
	return Path;
}

FString UAssetPathSubsystem::GetUEGeneratedAccentMeshesAbsPath() const
{
	TArray<FStringFormatArg> Args;
	Args.Add(FStringFormatArg(ProjectContentPlatformAbsPath));
	Args.Add(FStringFormatArg(PathConstants::MapMeshesDirName));
	Args.Add(FStringFormatArg(TEXT("/")));
	Args.Add(FStringFormatArg(PathConstants::ExportUEDirName));
	Args.Add(FStringFormatArg(TEXT("/")));
	Args.Add(FStringFormatArg(PathConstants::AccentMeshesDirName));
	Args.Add(FStringFormatArg(TEXT("/")));

	FString Path = FString::Format(TEXT("{0}{1}{2}{3}{4}{5}{6}"), Args);
	return Path;
}

FString UAssetPathSubsystem::GetImportedTerrainMeshesAbsPath() const
{
	TArray<FStringFormatArg> Args;
	Args.Add(FStringFormatArg(ProjectContentPlatformAbsPath));
	Args.Add(FStringFormatArg(PathConstants::MapMeshesDirName));
	Args.Add(FStringFormatArg(TEXT("/")));
	Args.Add(FStringFormatArg(PathConstants::ImportHoudiniDirName));
	Args.Add(FStringFormatArg(TEXT("/")));
	Args.Add(FStringFormatArg(PathConstants::TerrainMeshesDirName));
	Args.Add(FStringFormatArg(TEXT("/")));

	FString Path = FString::Format(TEXT("{0}{1}{2}{3}{4}{5}{6}"), Args);
	return Path;
}

FString UAssetPathSubsystem::GetImportedAccentMeshesAbsPath() const
{
	TArray<FStringFormatArg> Args;
	Args.Add(FStringFormatArg(ProjectContentPlatformAbsPath));
	Args.Add(FStringFormatArg(PathConstants::MapMeshesDirName));
	Args.Add(FStringFormatArg(TEXT("/")));
	Args.Add(FStringFormatArg(PathConstants::ImportHoudiniDirName));
	Args.Add(FStringFormatArg(TEXT("/")));
	Args.Add(FStringFormatArg(PathConstants::AccentMeshesDirName));
	Args.Add(FStringFormatArg(TEXT("/")));

	FString Path = FString::Format(TEXT("{0}{1}{2}{3}{4}{5}{6}"), Args);
	return Path;
}

// UE game asset path functions

FString UAssetPathSubsystem::GetUEGeneratedMeshesGamePath() const
{
	TArray<FStringFormatArg> Args;
	Args.Add(FStringFormatArg(PathConstants::MapMeshGameDir));
	Args.Add(FStringFormatArg(TEXT("/")));
	Args.Add(FStringFormatArg(PathConstants::ExportUEDirName));
	Args.Add(FStringFormatArg(TEXT("/")));

	FString Path = FString::Format(TEXT("{0}{1}{2}{3}"), Args);
	return Path;
}

FString UAssetPathSubsystem::GetUEGeneratedTerrainMeshesGamePath() const
{
	TArray<FStringFormatArg> Args;
	Args.Add(FStringFormatArg(GetUEGeneratedMeshesGamePath()));
	Args.Add(FStringFormatArg(PathConstants::TerrainMeshesDirName));
	Args.Add(FStringFormatArg(TEXT("/")));

	FString Path = FString::Format(TEXT("{0}{1}{2}"), Args);
	return Path;
}

FString UAssetPathSubsystem::GetUEGeneratedAccentMeshesGamePath() const
{
	TArray<FStringFormatArg> Args;
	Args.Add(FStringFormatArg(GetUEGeneratedMeshesGamePath()));
	Args.Add(FStringFormatArg(PathConstants::AccentMeshesDirName));
	Args.Add(FStringFormatArg(TEXT("/")));

	FString Path = FString::Format(TEXT("{0}{1}{2}"), Args);
	return Path;
}

FString UAssetPathSubsystem::GetImportedMeshesGamePath() const
{
	TArray<FStringFormatArg> Args;
	Args.Add(FStringFormatArg(PathConstants::MapMeshGameDir));
	Args.Add(FStringFormatArg(TEXT("/")));
	Args.Add(FStringFormatArg(PathConstants::ImportHoudiniDirName));
	Args.Add(FStringFormatArg(TEXT("/")));

	FString Path = FString::Format(TEXT("{0}{1}{2}{3}"), Args);
	return Path;
}

FString UAssetPathSubsystem::GetImportedTerrainMeshesGamePath() const
{
	TArray<FStringFormatArg> Args;
	Args.Add(FStringFormatArg(GetImportedMeshesGamePath()));
	Args.Add(FStringFormatArg(PathConstants::TerrainMeshesDirName));
	Args.Add(FStringFormatArg(TEXT("/")));

	FString Path = FString::Format(TEXT("{0}{1}{2}"), Args);
	return Path;
}

FString UAssetPathSubsystem::GetImportedAccentMeshesGamePath() const
{
	TArray<FStringFormatArg> Args;
	Args.Add(FStringFormatArg(GetImportedMeshesGamePath()));
	Args.Add(FStringFormatArg(PathConstants::AccentMeshesDirName));
	Args.Add(FStringFormatArg(TEXT("/")));

	FString Path = FString::Format(TEXT("{0}{1}{2}"), Args);
	return Path;
}

//--------------------------------------------------------------------------------------------

// Protected Functions --------------------------------------------------------------------------------------------

void UAssetPathSubsystem::SetupPathsAndFolders()
{
	FString ContentRelativePath = FPaths::ProjectContentDir();

	ProjectContentPlatformAbsPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*ContentRelativePath);

	if (UEditorAssetLibrary::DoesDirectoryExist(PathConstants::MapMeshGameDir))
	{
		UE_LOG(AutoMapMeshifyPathSubsystem, Display, TEXT("Auto map meshes directory already exists - checking for sub-direcories"));

		FString GeneratedMeshesDir = GetUEGeneratedMeshesGamePath();

		if (!UEditorAssetLibrary::DoesDirectoryExist(GeneratedMeshesDir))
		{
			CreateUEGeneratedMeshesDir();
		}
	
		FString ImportedMeshesGameDir = GetImportedMeshesGamePath();

		if (!UEditorAssetLibrary::DoesDirectoryExist(ImportedMeshesGameDir))
		{
			CreateImportedMeshesDir();
		}
	}
	else
	{
		UE_LOG(AutoMapMeshifyPathSubsystem, Warning, TEXT("Auto map mesh directory doesn't already exists - creating directories and sub-directories"));

		//Create map mesh directory in content browser
		bool bHasCreatedMapMeshDir = CreateAutoMapMeshDir();

		// If we have successfully created the map mash directory
		if (bHasCreatedMapMeshDir)
		{
			CreateUEGeneratedMeshesDir(); 
			CreateImportedMeshesDir();
			UE_LOG(AutoMapMeshifyPathSubsystem, Display, TEXT("Auto map mesh directory and subdirectories successfully created"));
		}
	}
}

bool UAssetPathSubsystem::CreateAutoMapMeshDir()
{
	UEditorAssetLibrary::MakeDirectory(PathConstants::MapMeshGameDir);
	bool bDoesMapMeshesDirExist = UEditorAssetLibrary::DoesDirectoryExist(PathConstants::MapMeshGameDir);

	if (bDoesMapMeshesDirExist)
	{
		UE_LOG(AutoMapMeshifyPathSubsystem, Display, TEXT("Successfully created auto map meshes directory"));
	} 
	else
	{
		UE_LOG(AutoMapMeshifyPathSubsystem, Warning, TEXT("Unsuccessfull in creating auto map meshes directory"));
	}

	return bDoesMapMeshesDirExist;
}

bool UAssetPathSubsystem::CreateUEGeneratedMeshesDir()
{
	FString GeneratedMeshesPath = GetUEGeneratedMeshesGamePath();

	UEditorAssetLibrary::MakeDirectory(GeneratedMeshesPath);
	bool bDoesGeneratedMeshesDirExist = UEditorAssetLibrary::DoesDirectoryExist(GeneratedMeshesPath);

	if (bDoesGeneratedMeshesDirExist)
	{
		UE_LOG(AutoMapMeshifyPathSubsystem, Display, TEXT("Auto map generated meshes directory successfully created/already exists so creating sub-directories"));
		
		FString GeneratedTerrainMeshesPath = GetUEGeneratedTerrainMeshesGamePath();
		FString GeneratedAccentMeshesPath = GetUEGeneratedAccentMeshesGamePath();

		UEditorAssetLibrary::MakeDirectory(GeneratedTerrainMeshesPath);
		UEditorAssetLibrary::MakeDirectory(GeneratedAccentMeshesPath);

		bool bDoesGeneratedTerrainMeshesDirExist = UEditorAssetLibrary::DoesDirectoryExist(GeneratedTerrainMeshesPath);
		bool bDoesGeneratedAccentMeshesDirExist = UEditorAssetLibrary::DoesDirectoryExist(GeneratedAccentMeshesPath);

		return bDoesGeneratedTerrainMeshesDirExist && bDoesGeneratedAccentMeshesDirExist;
	}
	else
	{
		UE_LOG(AutoMapMeshifyPathSubsystem, Warning, TEXT("Unsuccessfull in creating auto map generated meshes directory and sub directories"));
	}

	return bDoesGeneratedMeshesDirExist;
}

bool UAssetPathSubsystem::CreateImportedMeshesDir()
{
	FString ImporetedMeshesPath = GetImportedMeshesGamePath();

	UEditorAssetLibrary::MakeDirectory(ImporetedMeshesPath);
	bool bDoesGeneratedMeshesDirExist = UEditorAssetLibrary::DoesDirectoryExist(ImporetedMeshesPath);

	if (bDoesGeneratedMeshesDirExist)
	{
		UE_LOG(AutoMapMeshifyPathSubsystem, Display, TEXT("Auto map imported meshes directory successfully created/already exists so creating sub-directories"));

		FString ImportedTerrainMeshesPath = GetImportedTerrainMeshesGamePath();
		FString ImportedAccentMeshesPath = GetImportedAccentMeshesGamePath();

		UEditorAssetLibrary::MakeDirectory(ImportedTerrainMeshesPath);
		UEditorAssetLibrary::MakeDirectory(ImportedAccentMeshesPath);

		bool bDoesImportedTerrainMeshesDirExist = UEditorAssetLibrary::DoesDirectoryExist(ImportedTerrainMeshesPath);
		bool bDoesImportedAccentMeshesDirExist = UEditorAssetLibrary::DoesDirectoryExist(ImportedAccentMeshesPath);

		return bDoesImportedTerrainMeshesDirExist && bDoesImportedAccentMeshesDirExist;
	}
	else
	{
		UE_LOG(AutoMapMeshifyPathSubsystem, Warning, TEXT("Unsuccessfull in creating auto map imported meshes directory and sub directories"));
	}

	return bDoesGeneratedMeshesDirExist;
}

//--------------------------------------------------------------------------------------------
