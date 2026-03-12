// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "GameplayTagContainer.h"
#include "RawMesh.h"
#include "MeshManagerSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AutoMapMeshifyMeshManagerSubsystem, Log, All);

UENUM(meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EMeshActorType : uint8
{
	None = 0,
	UEStaticMesh = 1 << 0,
	UEHierInstancedStaticMesh = 1 << 1,
	AutoMapStaticMesh = 1 << 2,
	AutoMapHierInstancedStaticMesh = 1 << 3,
	Unknown = 1 << 4
};

ENUM_CLASS_FLAGS(EMeshActorType)

enum class EMapComponentType : uint8
{
	Terrain = 0,
	Accent,
};

struct FRawMeshValidityTracker
{
	FRawMeshValidityTracker()
		: bValidColors(false)
	{
		FMemory::Memset(bValidTexCoords, 0);
	}

	bool bValidTexCoords[MAX_MESH_TEXTURE_COORDS];
	bool bValidColors;
};


class UHierarchicalInstancedStaticMeshComponent;

/// <summary>
/// UMeshManagerSubsystem (EditorSubsystem)
///	
/// Use:
///		- Converts UE base mesh actors (static mesh or HISM actors) into their associated auto map mesh actor equivalents
///		- Converts auto map mesh actors into their associated UE base mesh actor equivalents
///		- Generates map meshes
/// </summary>
UCLASS()
class UMeshManagerSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void ConvertSelectedActors();

	void GetAllAutoMapMeshActors(TMap<FString, TArray<AActor*>>& OutAutoMapMeshActors);

	void ExecutePythonScript(FString PythonScript);

	void ConvertTaggedMapMeshActors();

	void ImportHoudiniExportMeshes();

protected:

	AActor* ConvertUEMeshActorToAMMeshActor(EMeshActorType eMeshActorType, AActor* pActorToConvert);
	
	AActor* ConvertAMMeshActorToUEMeshActor(EMeshActorType eMeshActorType, AActor* pActorToConvert);

	void SetupConvertedActorTransformAndLabel(EMeshActorType eSourceActorMeshType, AActor* pSourceActor, AActor* pTargetActor);

	EMeshActorType GetMeshActorType(AActor* pActor);

	bool SpawnConvertActorWindow(const FText& WindowTitle, const FText& WindowText);

	void GetSelectedActorsInViewport(TArray<AActor*>& OutActors);

	UWorld* GetCurrentWorld();

	void CopyMeshAndMaterials(AActor* pSourceActor, AActor* pTargetActor);

	void CopyMeshMaterialsAndInstanceTransforms(AActor* pSourceActor, AActor* pTargetActor);

	void GetSkinnedMeshFromActors(const TArray<AActor*>& InActors, TArray<UMeshComponent*>& OutMeshComponents);

	void StaticMeshToRawMeshes(UStaticMeshComponent* pInStaticMeshComponent, int32 InOverallMaxLODs, const FMatrix& InComponentToWorld, const FString& InPackageName, TArray<FRawMeshValidityTracker>& OutRawMeshTrackers, TArray<FRawMesh>& OutRawMeshes, TArray<UMaterialInterface*>& OutMaterials);
	
	void SkinnedMeshToRawMeshes(USkinnedMeshComponent* pInSkinnedMeshComponent, int32 InOverallMaxLODs, const FMatrix& InComponentToWorld, const FString& InPackageName, TArray<FRawMeshValidityTracker>& OutRawMeshTrackers, TArray<FRawMesh>& OutRawMeshes, TArray<UMaterialInterface*>& OutMaterials);
	
	void HierarchicalInstancedMeshToRawMeshes(UHierarchicalInstancedStaticMeshComponent* pInHISMComponent, int32 InOverallMaxLODs, const FTransform& InRootTransform, const FString& InPackageName, TArray<FRawMeshValidityTracker>& OutRawMeshTrackers, TArray<FRawMesh>& OutRawMeshes, TArray<UMaterialInterface*>& OutMaterials);

	template <typename ComponentType>
	void ProcessMaterials(ComponentType* pInComponent, const FString& InPackageName, TArray<UMaterialInterface*>& OutMaterials);

	void AddToMaterialList(UMaterialInterface* pInMaterialInterface, const FString& InPackageName, TArray<UMaterialInterface*>& OutMaterials);

	UStaticMesh* ConvertMeshComponentsToStaticMesh(const FString& MeshTag, const TArray<UMeshComponent*>& InMeshComponents, const FTransform& InRootTransform = FTransform::Identity, const FString& InPackageName = FString());

	void PopulateFilters();

	bool IsGameplayTagInFilter(const FGameplayTag& GameplayTagToCheck);
private:
	TSet<FGameplayTag> GameplaytagsFilter{};
};


