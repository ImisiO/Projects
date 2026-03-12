#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "AssetPathSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AutoMapMeshifyPathSubsystem, Log, All);

/// <summary>
/// UAssetPathSubsystem (EditorSubsystem)
///	
/// Use:
///		- Generates directories needed to generate the map meshes
///		- Generates directory string paths
/// </summary>
UCLASS()
class AUTOMAPMESHIFY_API UAssetPathSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/// <summary>
	/// Get absolute path of the projects content directory
	/// </summary>
	/// <returns>FString - Absolute path of the projects content directory</returns>
	/// <remarks>Keep in mind returned fstring will end in "/"</remarks>
	const FString& GetProjectContentAbsPath() const;

	/// <summary>
	/// Get absolute path of the ue generated 'terrain' meshes directory
	/// </summary>
	/// <returns>FString - Absolute path of the ue generated 'terrain' meshes</returns>
	/// <remarks>Keep in mind returned fstring will end in "/"</remarks>
	UFUNCTION(BlueprintCallable, Category = "AutoMapMeshify|Paths")
	FString GetUEGeneratedTerrainMeshesAbsPath() const;

	/// <summary>
	/// Get absolute path of the ue generated 'accent' meshes directory
	/// </summary>
	/// <returns>FString - Absolute path of the ue generated 'accent' meshes directory</returns>
	/// <remarks>Keep in mind returned fstring will end in "/"</remarks>
	UFUNCTION(BlueprintCallable, Category = "AutoMapMeshify|Paths")
	FString GetUEGeneratedAccentMeshesAbsPath() const;

	/// <summary>
	/// Get absolute path of the houdini generated 'terrain' meshes directory
	/// </summary>
	/// <returns>FString - Absolute path of the houdini generated 'terrain' meshes</returns>
	/// <remarks>Keep in mind returned fstring will end in "/"</remarks>
	UFUNCTION(BlueprintCallable, Category = "AutoMapMeshify|Paths")
	FString GetImportedTerrainMeshesAbsPath() const;

	/// <summary>
	/// Get absolute path of the houdini generated 'accent' meshes directory
	/// </summary>
	/// <returns>FString - Absolute path of the houdini generated 'accent' meshes directory</returns>
	/// <remarks>Keep in mind returned fstring will end in "/"</remarks>
	UFUNCTION(BlueprintCallable, Category = "AutoMapMeshify|Paths")
	FString GetImportedAccentMeshesAbsPath() const;

	/// <summary>
	/// Get UE project path of the generated 'terrain' meshes directory
	/// </summary>
	/// <returns>FString - UE project path of the generated meshes directory</returns>
	/// <remarks>Keep in mind returned fstring will end in "/"</remarks>
	UFUNCTION(BlueprintCallable, Category = "AutoMapMeshify|Paths")
	FString GetUEGeneratedMeshesGamePath() const;

	/// <summary>
	/// Get UE project path of the generated 'terrain' meshes directory
	/// </summary>
	/// <returns>FString - UE project path of the generated 'terrain' meshes directory</returns>
	/// <remarks>Keep in mind returned fstring will end in "/"</remarks>
	UFUNCTION(BlueprintCallable, Category = "AutoMapMeshify|Paths")
	FString GetUEGeneratedTerrainMeshesGamePath() const;

	/// <summary>
	/// Get UE project path of the generated 'accent' meshes directory
	/// </summary>
	/// <returns>FString - UE project path of the generated 'accent' meshes directory</returns>
	/// <remarks>Keep in mind returned fstring will end in "/"</remarks>
	UFUNCTION(BlueprintCallable, Category = "AutoMapMeshify|Paths")
	FString GetUEGeneratedAccentMeshesGamePath() const;

	/// <summary>
	/// Get UE project path of the generated 'terrain' meshes directory
	/// </summary>
	/// <returns>FString - UE project path of the meshes directory</returns>
	/// <remarks>Keep in mind returned fstring will end in "/"</remarks>
	UFUNCTION(BlueprintCallable, Category = "AutoMapMeshify|Paths")
	FString GetImportedMeshesGamePath() const;

	/// <summary>
	/// Get UE project path of the imported 'terrain' meshes directory
	/// </summary>
	/// <returns>FString - UE project path of the imported 'terrain' meshes directory</returns>
	/// <remarks>Keep in mind returned fstring will end in "/"</remarks>
	UFUNCTION(BlueprintCallable, Category = "AutoMapMeshify|Paths")
	FString GetImportedTerrainMeshesGamePath() const;

	/// <summary>
	/// Get UE project path of the 'accent' meshes directory
	/// </summary>
	/// <returns>FString - UE project path of the imported 'accent' meshes directory</returns>
	/// <remarks>Keep in mind returned fstring will end in "/"</remarks>
	UFUNCTION(BlueprintCallable, Category = "AutoMapMeshify|Paths")
	FString GetImportedAccentMeshesGamePath() const;

protected:

	/// <summary>
	/// Setup directories/sub directories/folders needed for the plugin to work as intended
	/// </summary>
	void SetupPathsAndFolders();

	/// <summary>
	/// Creates the "AutoMapMeshes" directory and its sub directories
	/// </summary>
	/// <returns>bool - Whether or not the directory was successfully created</returns>
	bool CreateAutoMapMeshDir();

	/// <summary>
	/// Creates the "GeneratedMeshes" (these are the meshes that are generated by the HDA) directory and its sub directories
	/// </summary>
	/// <returns>bool - Whether or not the directory was successfully created</returns>
	bool CreateUEGeneratedMeshesDir();

	/// <summary>
	/// Creates the "ImportedMeshes" directory and its sub directories
	/// </summary>
	/// <returns>bool - Whether or not the directory was successfully created</returns>
	bool CreateImportedMeshesDir();

	/// <summary>
	/// Absolute path to the current projects content directory
	/// </summary>
	FString ProjectContentPlatformAbsPath = FString(TEXT(""));
};

namespace PathConstants
{
	const FString BaseGameDir = FString(TEXT("/Game/"));
	const FString MapMeshesDirName = FString(TEXT("AutoMapMeshes"));
	const FString ImportHoudiniDirName = FString(TEXT("HoudiniImport"));
	const FString ExportUEDirName = FString(TEXT("UEExport"));
	const FString TerrainMeshesDirName = FString(TEXT("Terrain"));
	const FString AccentMeshesDirName = FString(TEXT("Accents"));
	const FString MapMeshGameDir = PathConstants::BaseGameDir + PathConstants::MapMeshesDirName;
}// namespace PathConstants