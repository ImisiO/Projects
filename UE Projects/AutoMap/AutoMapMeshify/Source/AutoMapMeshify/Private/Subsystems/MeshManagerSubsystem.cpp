#include "Subsystems/MeshManagerSubsystem.h"

#include "Logging/StructuredLog.h"
#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"

#include "EditorAssetLibrary.h"
#include "ActorEditorUtils.h"
#include "ActorGroupingUtils.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Engine.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectGlobals.h"
#include "EditorDirectories.h"

#include "Dialogs/Dialogs.h"
#include "Dialogs/DlgPickAssetPath.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Framework/Notifications/NotificationManager.h"

#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformMath.h"

#include "ContentBrowserModule.h"
#include "ContentBrowserDataSubsystem.h"
#include "IContentBrowserDataModule.h"
#include "IContentBrowserSingleton.h"
#include "Settings/ContentBrowserSettings.h"
#include "AssetToolsModule.h"

#include "AssetRegistry/AssetRegistryModule.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/VolumetricCloudComponent.h"

#include "GameplayTagsManager.h"
#include "MRUFavoritesList.h"

#include "IPythonScriptPlugin.h"

#include "SkeletalRenderPublic.h"
#include "Rendering/SkeletalMeshRenderData.h"

#include "Materials/MaterialInstance.h"

#include "AutoMapHISMActor.h"
#include "AutoMapSMActor.h"

DEFINE_LOG_CATEGORY(AutoMapMeshifyMeshManagerSubsystem);

#define LOCTEXT_NAMESPACE "AutoMapMeshifyMeshManagerSubsytem"

// Public Functions --------------------------------------------------------------------------------------------

void UMeshManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	this->SetFlags(GetFlags() | RF_Transactional);

	PopulateFilters();
}

void UMeshManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UMeshManagerSubsystem::ConvertSelectedActors()
{
	TArray<AActor*> SelectedActors{};
	TArray<AActor*> NewConvertedActors{};

	TArray<AActor*> UEMeshActorsToConvert{};
	TArray<AActor*> AutoMapMeshActorsToConvert{};

	GetSelectedActorsInViewport(SelectedActors);

	int32 NumberOfSelectedActors = SelectedActors.Num();
	UE_LOGFMT(AutoMapMeshifyMeshManagerSubsystem, Log, "Number of Selected Actors: {0}", NumberOfSelectedActors);

	for (AActor* pSelectedActor : SelectedActors)
	{
		if (pSelectedActor)
		{
			EMeshActorType eMeshActorType = GetMeshActorType(pSelectedActor);
			
			constexpr EMeshActorType UEMeshActorFlags = EMeshActorType::UEStaticMesh | EMeshActorType::UEHierInstancedStaticMesh;
			constexpr EMeshActorType AutoMapMeshActorFlags = EMeshActorType::AutoMapStaticMesh | EMeshActorType::AutoMapHierInstancedStaticMesh;

			if (EnumHasAnyFlags(eMeshActorType, UEMeshActorFlags))
			{
				UEMeshActorsToConvert.Add(pSelectedActor);
			}
			else if (EnumHasAnyFlags(eMeshActorType, AutoMapMeshActorFlags))
			{
				AutoMapMeshActorsToConvert.Add(pSelectedActor);
			}
		}
	}

	int32 NumberOfSelectedUEMeshActors = UEMeshActorsToConvert.Num();
	int32 NumberOfSelectedAutoMapMeshActors = AutoMapMeshActorsToConvert.Num();

	FText DialogTitle = FText::FromString("[AutoMapMeshify] Convert Selected Actors");
	
	bool bDoesUserWantToConvertUEMeshActors = false;
	bool bDoesUserWantToConvertAMMeshActors = false;

	if (NumberOfSelectedUEMeshActors > 0)
	{
		FText UEDialogMessage = FText::Format(
			FText::FromString("You are about to convert {0} selected UE mesh actor(s) into AutoMap mesh actor(s).\nThis action can be undone. Do you want to proceed?"),
			FText::AsNumber(NumberOfSelectedUEMeshActors)
		);
		bDoesUserWantToConvertUEMeshActors = SpawnConvertActorWindow(DialogTitle, UEDialogMessage);
	}

	if (NumberOfSelectedAutoMapMeshActors > 0)
	{
		FText AMDialogMessage = FText::Format(
			FText::FromString("You are about to convert {0} selected AutoMap mesh actor(s) into UE mesh actor(s).\nThis action can be undone. Do you want to proceed?"),
			FText::AsNumber(NumberOfSelectedAutoMapMeshActors)
		);
		bDoesUserWantToConvertAMMeshActors = SpawnConvertActorWindow(DialogTitle, AMDialogMessage);
	}

	if (!bDoesUserWantToConvertUEMeshActors && !bDoesUserWantToConvertAMMeshActors)
	{
		return;
	}

	// Begin transaction so conversion can be undone
	UKismetSystemLibrary::BeginTransaction("AutoMapMeshifyMeshManagerSubsystem", FText::FromString("Covert Static Mesh Actors Action"), nullptr);

	if (bDoesUserWantToConvertUEMeshActors)
	{
		for (AActor* pUEMeshActor : UEMeshActorsToConvert)
		{
			AActor* pNewAutoMapMeshActor = ConvertUEMeshActorToAMMeshActor(GetMeshActorType(pUEMeshActor), pUEMeshActor);

			if (pNewAutoMapMeshActor)
			{
				NewConvertedActors.Add(pNewAutoMapMeshActor);
			}
		}
	}

	if (bDoesUserWantToConvertAMMeshActors)
	{
		for (AActor* pAMMeshActor : AutoMapMeshActorsToConvert)
		{
			AActor* pNewUEMeshActor = ConvertAMMeshActorToUEMeshActor(GetMeshActorType(pAMMeshActor), pAMMeshActor);

			if (pNewUEMeshActor)
			{
				NewConvertedActors.Add(pNewUEMeshActor);
			}
		}
	}

	GUnrealEd->GetSelectedActors()->Modify();

	GUnrealEd->GetSelectedActors()->BeginBatchSelectOperation();

	GEditor->SelectNone(true, true);

	for (AActor* pActor : NewConvertedActors)
	{
		GUnrealEd->SelectActor(pActor, true, true);
	}

	GUnrealEd->GetSelectedActors()->EndBatchSelectOperation();

	UKismetSystemLibrary::EndTransaction();
}

void UMeshManagerSubsystem::GetAllAutoMapMeshActors(TMap<FString, TArray<AActor*>>& OutAutoMapMeshActors)
{
	TArray<AActor*> FoundAutoMapMeshActors;
	
	UWorld* pWorld = GetCurrentWorld();

	if (pWorld)
	{
		UGameplayStatics::GetAllActorsOfClass(pWorld, AAutoMapSMActor::StaticClass(), FoundAutoMapMeshActors);

		for (AActor* pAutoMapMeshActor : FoundAutoMapMeshActors)
		{
			if (pAutoMapMeshActor)
			{
				AAutoMapSMActor* pAutoMapSMActor = Cast<AAutoMapSMActor>(pAutoMapMeshActor);

				if (pAutoMapSMActor)
				{
					const FGameplayTag& MeshfiyTag = pAutoMapSMActor->GetMeshCombineTag();
					FString MeshifyTagString = MeshfiyTag.ToString();

					if (OutAutoMapMeshActors.Contains(MeshifyTagString))
					{
						OutAutoMapMeshActors[MeshifyTagString].Add(pAutoMapSMActor);
					}
					else
					{
						OutAutoMapMeshActors.Add(MeshifyTagString, { pAutoMapSMActor });
					}
				}
			}
		}

		UGameplayStatics::GetAllActorsOfClass(pWorld, AAutoMapHISMActor::StaticClass(), FoundAutoMapMeshActors);

		for (AActor* pAutoMapMeshActor : FoundAutoMapMeshActors)
		{
			if (pAutoMapMeshActor)
			{
				AAutoMapHISMActor* pAutoMapHISMActor = Cast<AAutoMapHISMActor>(pAutoMapMeshActor);

				if (pAutoMapHISMActor)
				{
					const FGameplayTag& MeshfiyTag = pAutoMapHISMActor->GetMeshCombineTag();
					FString MeshifyTagString = MeshfiyTag.ToString();

					if (OutAutoMapMeshActors.Contains(MeshifyTagString))
					{
						OutAutoMapMeshActors[MeshifyTagString].Add(pAutoMapHISMActor);
					}
					else
					{
						OutAutoMapMeshActors.Add(MeshifyTagString, { pAutoMapHISMActor });
					}
				}
			}
		}
	}
}

void UMeshManagerSubsystem::ExecutePythonScript(FString PythonScript)
{
	IPythonScriptPlugin::Get()->ExecPythonCommand(*PythonScript);
}

void UMeshManagerSubsystem::ConvertTaggedMapMeshActors()
{
	TMap<FString, TArray<AActor*>> TaggedActors = TMap<FString, TArray<AActor*>>();

	GetAllAutoMapMeshActors(TaggedActors);

	for (auto& TagToActorsList : TaggedActors)
	{
		// Exclude untagged actors
		if (!TagToActorsList.Key.Equals(TEXT("None")))
		{
			TArray<UMeshComponent*> MeshComponents;

			GetSkinnedMeshFromActors(TagToActorsList.Value, MeshComponents);

			auto GetActorRootTransform = [](AActor* InActor)
				{
					FTransform RootTransform(FTransform::Identity);
					if (ACharacter* Character = Cast<ACharacter>(InActor))
					{
						RootTransform = Character->GetTransform();
						RootTransform.SetLocation(RootTransform.GetLocation() - FVector(0.0f, 0.0f, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
					}
					else
					{
						// otherwise just use the actor's origin
						RootTransform = InActor->GetTransform();
					}

					return RootTransform;
				};

			// now pick a root transform
			FTransform RootTransform(FTransform::Identity);
			if (TagToActorsList.Value.Num() == 1)
			{
				RootTransform = GetActorRootTransform(TagToActorsList.Value[0]);
			}
			else
			{
				// multiple actors use the average of their origins, with Z being the min of all origins. Rotation is identity for simplicity
				FVector Location(FVector::ZeroVector);
				double MinZ = DBL_MAX;
				for (AActor* Actor : TagToActorsList.Value)
				{
					FTransform ActorTransform(GetActorRootTransform(Actor));
					Location += ActorTransform.GetLocation();
					MinZ = FMath::Min(ActorTransform.GetLocation().Z, MinZ);
				}
				Location /= (float)TagToActorsList.Value.Num();
				Location.Z = MinZ;

				RootTransform.SetLocation(Location);
			}

			UStaticMesh* StaticMesh = ConvertMeshComponentsToStaticMesh(TagToActorsList.Key, MeshComponents, RootTransform);

			// Also notify the content browser that the new assets exists
			if (StaticMesh)
			{
				FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
				ContentBrowserModule.Get().SyncBrowserToAssets(TArray<UObject*>({ StaticMesh }), true);
			}
		}
	}

	FString Command = FString(TEXT("ExportUEGeneratedMeshes()"));
	ExecutePythonScript(Command);
}

void UMeshManagerSubsystem::ImportHoudiniExportMeshes()
{
	FString Command = FString(TEXT("ImportHoudiniMeshExports()"));
	ExecutePythonScript(Command);
}

//--------------------------------------------------------------------------------------------

// Protected Functions --------------------------------------------------------------------------------------------

AActor* UMeshManagerSubsystem::ConvertUEMeshActorToAMMeshActor(EMeshActorType eMeshActorType, AActor* pActorToConvert)
{
	AActor* pSpawnedActor = nullptr;
	UWorld* pWorld = GetCurrentWorld();

	if (pActorToConvert && pWorld)
	{
		FActorSpawnParameters SpawnInfo;

		switch (eMeshActorType)
		{
		case EMeshActorType::UEStaticMesh:
			pSpawnedActor = pWorld->SpawnActor<AAutoMapSMActor>(AAutoMapSMActor::StaticClass(), FVector(), FRotator(), SpawnInfo);

			if (pSpawnedActor)
			{
				CopyMeshAndMaterials(pActorToConvert, pSpawnedActor);
			}
			break;

		case EMeshActorType::UEHierInstancedStaticMesh:
			pSpawnedActor = pWorld->SpawnActor<AAutoMapHISMActor>(AAutoMapHISMActor::StaticClass(), FVector(), FRotator(), SpawnInfo);
				
			if (pSpawnedActor)
			{
				CopyMeshMaterialsAndInstanceTransforms(pActorToConvert, pSpawnedActor);
			}
			break;

		default:
			break;
		}
	}

	if (pSpawnedActor)
	{
		SetupConvertedActorTransformAndLabel(eMeshActorType, pActorToConvert, pSpawnedActor);
	}

	return pSpawnedActor;
}

AActor* UMeshManagerSubsystem::ConvertAMMeshActorToUEMeshActor(EMeshActorType eMeshActorType, AActor* pActorToConvert)
{
	AActor* pSpawnedActor = nullptr;
	UWorld* pWorld = GetCurrentWorld();

	if (pActorToConvert && pWorld)
	{
		FActorSpawnParameters SpawnInfo;

		switch (eMeshActorType)
		{
		case EMeshActorType::AutoMapStaticMesh:
			pSpawnedActor = pWorld->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector(), FRotator(), SpawnInfo);

			if (pSpawnedActor)
			{
				CopyMeshAndMaterials(pActorToConvert, pSpawnedActor);
			}

			break;

		case EMeshActorType::AutoMapHierInstancedStaticMesh:
			pSpawnedActor = pWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(), FRotator(), SpawnInfo);

			if (pSpawnedActor)
			{
				UHierarchicalInstancedStaticMeshComponent* HISMComponent =
					NewObject<UHierarchicalInstancedStaticMeshComponent>(
						pSpawnedActor,
						UHierarchicalInstancedStaticMeshComponent::StaticClass(),
						TEXT("HISMComponent")
					);

				HISMComponent->SetupAttachment(pSpawnedActor->GetRootComponent());
				HISMComponent->RegisterComponent();

				CopyMeshMaterialsAndInstanceTransforms(pActorToConvert, pSpawnedActor);
			}
			break;

		default:
			break;
		}
	}

	if (pSpawnedActor)
	{
		SetupConvertedActorTransformAndLabel(eMeshActorType, pActorToConvert, pSpawnedActor);
	}

	return pSpawnedActor;
}

void UMeshManagerSubsystem::SetupConvertedActorTransformAndLabel(EMeshActorType eSourceActorMeshType, AActor* pSourceActor, AActor* pTargetActor)
{
	if (pSourceActor && pTargetActor)
	{
		constexpr EMeshActorType UEMeshActorFlags = EMeshActorType::UEStaticMesh | EMeshActorType::UEHierInstancedStaticMesh;
		constexpr EMeshActorType AutoMapMeshActorFlags = EMeshActorType::AutoMapStaticMesh | EMeshActorType::AutoMapHierInstancedStaticMesh;

		FTransform StaticMeshTransform = pSourceActor->GetTransform();

		pTargetActor->SetActorTransform(StaticMeshTransform);

		if (EnumHasAnyFlags(eSourceActorMeshType, UEMeshActorFlags))
		{
			FString NewActorLabel = pSourceActor->GetActorLabel() + FString(TEXT("_AutoMap"));
			pTargetActor->SetActorLabel(NewActorLabel);
		}
		else if (EnumHasAnyFlags(eSourceActorMeshType, AutoMapMeshActorFlags))
		{
			FString NewActorLabel = pSourceActor->GetActorLabel();
			NewActorLabel = NewActorLabel.Replace(TEXT("_AutoMap"), TEXT(""));
			pTargetActor->SetActorLabel(NewActorLabel);
		}

		pTargetActor->SetFolderPath(pSourceActor->GetFolderPath());

		pSourceActor->Destroy();
	} 
	else
	{
		return;
	}
}

EMeshActorType UMeshManagerSubsystem::GetMeshActorType(AActor* pActor)
{
	if (!pActor) 
	{
		return EMeshActorType::Unknown;
	}

	UHierarchicalInstancedStaticMeshComponent* pHISMComp = pActor->FindComponentByClass<UHierarchicalInstancedStaticMeshComponent>();
	AAutoMapHISMActor* pAutoMapHISM = Cast<AAutoMapHISMActor>(pActor);
	AAutoMapSMActor* pAutoMapSM = Cast<AAutoMapSMActor>(pActor);
	AStaticMeshActor* pStaticMesh = Cast<AStaticMeshActor>(pActor);

	if (pHISMComp && pAutoMapHISM)  return EMeshActorType::AutoMapHierInstancedStaticMesh;
	if (pHISMComp)                  return EMeshActorType::UEHierInstancedStaticMesh;
	if (pAutoMapSM)                 return EMeshActorType::AutoMapStaticMesh;
	if (pStaticMesh)                return EMeshActorType::UEStaticMesh;

	return EMeshActorType::Unknown;
}

bool UMeshManagerSubsystem::SpawnConvertActorWindow(const FText& WindowTitle, const FText& WindowText)
{
	EAppReturnType::Type UserResponse = FMessageDialog::Open(EAppMsgType::YesNo, WindowText, WindowTitle);

	if (UserResponse != EAppReturnType::Yes)
	{
		return false;
	}
	else 
	{
		return true;
	}
}

void UMeshManagerSubsystem::GetSelectedActorsInViewport(TArray<AActor*>& OutActors)
{
	TArray<UObject*> UActorObjects{};

	GUnrealEd->GetSelectedActors()->GetSelectedObjects(AActor::StaticClass(), UActorObjects);

	for (UObject* pObject : UActorObjects)
	{
		AActor* pActor = Cast<AActor>(pObject);

		if (pActor)
		{
			OutActors.Add(pActor);
		}
	}
}

UWorld* UMeshManagerSubsystem::GetCurrentWorld()
{
	if (GEngine)
	{
		FWorldContext* WorldContext = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);

		if (WorldContext)
		{
			return WorldContext->World();
		}
	}

	return nullptr;
}

void UMeshManagerSubsystem::CopyMeshAndMaterials(AActor* pSourceActor, AActor* pTargetActor)
{
	if (!pSourceActor || !pTargetActor)
	{
		return;
	}

	UStaticMeshComponent* SourceMeshComp = pSourceActor->FindComponentByClass<UStaticMeshComponent>();
	UStaticMeshComponent* TargetMeshComp = pTargetActor->FindComponentByClass<UStaticMeshComponent>();

	if (SourceMeshComp && TargetMeshComp)
	{
		UStaticMesh* Mesh = SourceMeshComp->GetStaticMesh();
		TargetMeshComp->SetStaticMesh(Mesh);

		int32 MaterialCount = SourceMeshComp->GetNumMaterials();
		for (int32 i = 0; i < MaterialCount; ++i)
		{
			UMaterialInterface* SourceMaterial = SourceMeshComp->GetMaterial(i);
			TargetMeshComp->SetMaterial(i, SourceMaterial);
		}

		TargetMeshComp->CopyInstanceVertexColorsIfCompatible(SourceMeshComp);
	}
}

void UMeshManagerSubsystem::CopyMeshMaterialsAndInstanceTransforms(AActor* pSourceActor, AActor* pTargetActor)
{
	if (!pSourceActor || !pTargetActor)
	{
		return;
	}

	UHierarchicalInstancedStaticMeshComponent* SourceHISMComp = pSourceActor->FindComponentByClass<UHierarchicalInstancedStaticMeshComponent>();
	UHierarchicalInstancedStaticMeshComponent* TargetHISMComp = pTargetActor->FindComponentByClass<UHierarchicalInstancedStaticMeshComponent>();

	if (SourceHISMComp && TargetHISMComp)
	{
		UStaticMesh* Mesh = SourceHISMComp->GetStaticMesh();
		TargetHISMComp->SetStaticMesh(Mesh);

		int32 MaterialCount = SourceHISMComp->GetNumMaterials();
		for (int32 i = 0; i < MaterialCount; ++i)
		{
			UMaterialInterface* SourceMaterial = SourceHISMComp->GetMaterial(i);
			TargetHISMComp->SetMaterial(i, SourceMaterial);
		}

		int32 NumberOfTransforms = SourceHISMComp->GetInstanceCount();

		for (int InstanceIndex = 0; InstanceIndex < NumberOfTransforms; InstanceIndex++)
		{
			FTransform SourceInstanceTransform;
			SourceHISMComp->GetInstanceTransform(InstanceIndex, SourceInstanceTransform, false);
			TargetHISMComp->AddInstance(SourceInstanceTransform, false);
		}
	}
}

void UMeshManagerSubsystem::GetSkinnedMeshFromActors(const TArray<AActor*>& InActors, TArray<UMeshComponent*>& OutMeshComponents)
{
	for (AActor* Actor : InActors)
	{
		// add all components from this actor
		TInlineComponentArray<UMeshComponent*> ActorComponents(Actor);
		for (UMeshComponent* ActorComponent : ActorComponents)
		{
			if (ActorComponent->IsA(USkinnedMeshComponent::StaticClass()) || ActorComponent->IsA(UStaticMeshComponent::StaticClass()) || ActorComponent->IsA(UHierarchicalInstancedStaticMeshComponent::StaticClass()))
			{
				OutMeshComponents.AddUnique(ActorComponent);
			}
		}

		// add all attached actors
		TArray<AActor*> AttachedActors;
		Actor->GetAttachedActors(AttachedActors);
		for (AActor* AttachedActor : AttachedActors)
		{
			TInlineComponentArray<UMeshComponent*> AttachedActorComponents(AttachedActor);
			for (UMeshComponent* AttachedActorComponent : AttachedActorComponents)
			{
				if (AttachedActorComponent->IsA(USkinnedMeshComponent::StaticClass()) || AttachedActorComponent->IsA(UStaticMeshComponent::StaticClass()) || AttachedActorComponent->IsA(UHierarchicalInstancedStaticMeshComponent::StaticClass()))
				{
					OutMeshComponents.AddUnique(AttachedActorComponent);
				}
			}
		}
	}
}

void UMeshManagerSubsystem::StaticMeshToRawMeshes(UStaticMeshComponent* pInStaticMeshComponent, int32 InOverallMaxLODs, const FMatrix& InComponentToWorld, const FString& InPackageName, TArray<FRawMeshValidityTracker>& OutRawMeshTrackers, TArray<FRawMesh>& OutRawMeshes, TArray<UMaterialInterface*>& OutMaterials)
{
	const int32 BaseMaterialIndex = OutMaterials.Num();

	const int32 NumLODs = pInStaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources.Num();

	for (int32 OverallLODIndex = 0; OverallLODIndex < InOverallMaxLODs; OverallLODIndex++)
	{
		int32 LODIndexRead = FMath::Min(OverallLODIndex, NumLODs - 1);

		FRawMesh& RawMesh = OutRawMeshes[OverallLODIndex];
		FRawMeshValidityTracker& RawMeshTracker = OutRawMeshTrackers[OverallLODIndex];
		const FStaticMeshLODResources& LODResource = pInStaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources[LODIndexRead];
		const int32 BaseVertexIndex = RawMesh.VertexPositions.Num();

		for (int32 VertIndex = 0; VertIndex < LODResource.GetNumVertices(); ++VertIndex)
		{
			RawMesh.VertexPositions.Add(FVector4f(InComponentToWorld.TransformPosition((FVector)LODResource.VertexBuffers.PositionVertexBuffer.VertexPosition((uint32)VertIndex))));
		}

		const FIndexArrayView IndexArrayView = LODResource.IndexBuffer.GetArrayView();
		const FStaticMeshVertexBuffer& StaticMeshVertexBuffer = LODResource.VertexBuffers.StaticMeshVertexBuffer;
		const int32 NumTexCoords = FMath::Min(StaticMeshVertexBuffer.GetNumTexCoords(), (uint32)MAX_MESH_TEXTURE_COORDS);
		const int32 NumSections = LODResource.Sections.Num();

		for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
		{
			const FStaticMeshSection& StaticMeshSection = LODResource.Sections[SectionIndex];

			const int32 NumIndices = StaticMeshSection.NumTriangles * 3;
			for (int32 IndexIndex = 0; IndexIndex < NumIndices; IndexIndex++)
			{
				int32 Index = IndexArrayView[StaticMeshSection.FirstIndex + IndexIndex];
				RawMesh.WedgeIndices.Add(BaseVertexIndex + Index);

				RawMesh.WedgeTangentX.Add(FVector4f(InComponentToWorld.TransformVector(FVector(StaticMeshVertexBuffer.VertexTangentX(Index)))));
				RawMesh.WedgeTangentY.Add(FVector4f(InComponentToWorld.TransformVector(FVector(StaticMeshVertexBuffer.VertexTangentY(Index)))));
				RawMesh.WedgeTangentZ.Add(FVector4f(InComponentToWorld.TransformVector(FVector(StaticMeshVertexBuffer.VertexTangentZ(Index)))));

				for (int32 TexCoordIndex = 0; TexCoordIndex < MAX_MESH_TEXTURE_COORDS; TexCoordIndex++)
				{
					if (TexCoordIndex >= NumTexCoords)
					{
						RawMesh.WedgeTexCoords[TexCoordIndex].AddDefaulted();
					}
					else
					{
						RawMesh.WedgeTexCoords[TexCoordIndex].Add(StaticMeshVertexBuffer.GetVertexUV(Index, TexCoordIndex));
						RawMeshTracker.bValidTexCoords[TexCoordIndex] = true;
					}
				}

				if (LODResource.VertexBuffers.ColorVertexBuffer.IsInitialized())
				{
					RawMesh.WedgeColors.Add(LODResource.VertexBuffers.ColorVertexBuffer.VertexColor(Index));
					RawMeshTracker.bValidColors = true;
				}
				else
				{
					RawMesh.WedgeColors.Add(FColor::White);
				}
			}

			// copy face info
			for (uint32 TriIndex = 0; TriIndex < StaticMeshSection.NumTriangles; TriIndex++)
			{
				RawMesh.FaceMaterialIndices.Add(BaseMaterialIndex + StaticMeshSection.MaterialIndex);
				RawMesh.FaceSmoothingMasks.Add(0); // Assume this is ignored as bRecomputeNormals is false
			}
		}
	}

	//ProcessMaterials<UStaticMeshComponent>(pInStaticMeshComponent, InPackageName, OutMaterials);
}

void UMeshManagerSubsystem::SkinnedMeshToRawMeshes(USkinnedMeshComponent* pInSkinnedMeshComponent, int32 InOverallMaxLODs, const FMatrix& InComponentToWorld, const FString& InPackageName, TArray<FRawMeshValidityTracker>& OutRawMeshTrackers, TArray<FRawMesh>& OutRawMeshes, TArray<UMaterialInterface*>& OutMaterials)
{
	const int32 BaseMaterialIndex = OutMaterials.Num();

	// Export all LODs to raw meshes
	const int32 NumLODs = pInSkinnedMeshComponent->GetNumLODs();
	//The cpu skinned vertice is not valid under min lod
	int32 MinLOD = pInSkinnedMeshComponent->ComputeMinLOD();

	for (int32 OverallLODIndex = MinLOD; OverallLODIndex < InOverallMaxLODs; OverallLODIndex++)
	{
		int32 LODIndexRead = FMath::Min(OverallLODIndex, NumLODs - 1);

		FRawMesh& RawMesh = OutRawMeshes[OverallLODIndex];
		FRawMeshValidityTracker& RawMeshTracker = OutRawMeshTrackers[OverallLODIndex];
		const int32 BaseVertexIndex = RawMesh.VertexPositions.Num();

		FSkeletalMeshLODInfo& SrcLODInfo = *(pInSkinnedMeshComponent->GetSkinnedAsset()->GetLODInfo(LODIndexRead));

		// Get the CPU skinned verts for this LOD
		TArray<FFinalSkinVertex> FinalVertices;
		pInSkinnedMeshComponent->GetCPUSkinnedVertices(FinalVertices, LODIndexRead);

		FSkeletalMeshRenderData& SkeletalMeshRenderData = pInSkinnedMeshComponent->MeshObject->GetSkeletalMeshRenderData();
		FSkeletalMeshLODRenderData& LODData = SkeletalMeshRenderData.LODRenderData[LODIndexRead];

		// Copy skinned vertex positions
		for (int32 VertIndex = 0; VertIndex < FinalVertices.Num(); ++VertIndex)
		{
			RawMesh.VertexPositions.Add((FVector4f)InComponentToWorld.TransformPosition((FVector)FinalVertices[VertIndex].Position));
		}

		const uint32 NumTexCoords = FMath::Min(LODData.StaticVertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords(), (uint32)MAX_MESH_TEXTURE_COORDS);
		const int32 NumSections = LODData.RenderSections.Num();
		FRawStaticIndexBuffer16or32Interface& IndexBuffer = *LODData.MultiSizeIndexContainer.GetIndexBuffer();

		for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
		{
			const FSkelMeshRenderSection& SkelMeshSection = LODData.RenderSections[SectionIndex];
			if (pInSkinnedMeshComponent->IsMaterialSectionShown(SkelMeshSection.MaterialIndex, LODIndexRead))
			{
				// Build 'wedge' info
				const int32 NumWedges = SkelMeshSection.NumTriangles * 3;
				for (int32 WedgeIndex = 0; WedgeIndex < NumWedges; WedgeIndex++)
				{
					const int32 VertexIndexForWedge = IndexBuffer.Get(SkelMeshSection.BaseIndex + WedgeIndex);

					RawMesh.WedgeIndices.Add(BaseVertexIndex + VertexIndexForWedge);

					const FFinalSkinVertex& SkinnedVertex = FinalVertices[VertexIndexForWedge];
					const FVector3f TangentX = (FVector4f)InComponentToWorld.TransformVector(SkinnedVertex.TangentX.ToFVector());
					const FVector3f TangentZ = (FVector4f)InComponentToWorld.TransformVector(SkinnedVertex.TangentZ.ToFVector());
					const FVector4 UnpackedTangentZ = SkinnedVertex.TangentZ.ToFVector4();
					const FVector3f TangentY = (TangentZ ^ TangentX).GetSafeNormal() * UnpackedTangentZ.W;

					RawMesh.WedgeTangentX.Add(TangentX);
					RawMesh.WedgeTangentY.Add(TangentY);
					RawMesh.WedgeTangentZ.Add(TangentZ);

					for (uint32 TexCoordIndex = 0; TexCoordIndex < MAX_MESH_TEXTURE_COORDS; TexCoordIndex++)
					{
						if (TexCoordIndex >= NumTexCoords)
						{
							RawMesh.WedgeTexCoords[TexCoordIndex].AddDefaulted();
						}
						else
						{
							RawMesh.WedgeTexCoords[TexCoordIndex].Add(LODData.StaticVertexBuffers.StaticMeshVertexBuffer.GetVertexUV(VertexIndexForWedge, TexCoordIndex));
							RawMeshTracker.bValidTexCoords[TexCoordIndex] = true;
						}
					}

					if (LODData.StaticVertexBuffers.ColorVertexBuffer.IsInitialized() && LODData.StaticVertexBuffers.ColorVertexBuffer.GetNumVertices() > 0)
					{
						RawMesh.WedgeColors.Add(LODData.StaticVertexBuffers.ColorVertexBuffer.VertexColor(VertexIndexForWedge));
						RawMeshTracker.bValidColors = true;
					}
					else
					{
						RawMesh.WedgeColors.Add(FColor::White);
					}
				}

				int32 MaterialIndex = SkelMeshSection.MaterialIndex;
				// use the remapping of material indices if there is a valid value
				if (SrcLODInfo.LODMaterialMap.IsValidIndex(SectionIndex) && SrcLODInfo.LODMaterialMap[SectionIndex] != INDEX_NONE)
				{
					MaterialIndex = FMath::Clamp<int32>(SrcLODInfo.LODMaterialMap[SectionIndex], 0, pInSkinnedMeshComponent->GetSkinnedAsset()->GetMaterials().Num() - 1);
				}

				// copy face info
				for (uint32 TriIndex = 0; TriIndex < SkelMeshSection.NumTriangles; TriIndex++)
				{
					RawMesh.FaceMaterialIndices.Add(BaseMaterialIndex + MaterialIndex);
					RawMesh.FaceSmoothingMasks.Add(0); // Assume this is ignored as bRecomputeNormals is false
				}
			}
		}
	}

	//ProcessMaterials<USkinnedMeshComponent>(pInSkinnedMeshComponent, InPackageName, OutMaterials);
}

void UMeshManagerSubsystem::HierarchicalInstancedMeshToRawMeshes(UHierarchicalInstancedStaticMeshComponent* pInHISMComponent, int32 InOverallMaxLODs, const FTransform& InRootTransform, const FString& InPackageName, TArray<FRawMeshValidityTracker>& OutRawMeshTrackers, TArray<FRawMesh>& OutRawMeshes, TArray<UMaterialInterface*>& OutMaterials)
{
	const int32 BaseMaterialIndex = OutMaterials.Num();

	const int32 NumLODs = pInHISMComponent->GetStaticMesh()->GetRenderData()->LODResources.Num();

	const int32 NumInstances = pInHISMComponent->GetInstanceCount();

	TArray<FMatrix> InstanceMatrices;

	InstanceMatrices.SetNum(NumInstances);

	for (int32 InstanceIndex = 0; InstanceIndex < NumInstances; ++InstanceIndex)
	{
		FMatrix& InstanceMatrix = InstanceMatrices[InstanceIndex];

		FTransform InstanceTransform;
		bool bHasAquiredTransformSuccessfully = pInHISMComponent->GetInstanceTransform(InstanceIndex, InstanceTransform, true);

		if (bHasAquiredTransformSuccessfully)
		{
			FTransform RootTransformPivotOnly(InRootTransform.GetRotation(), InRootTransform.GetLocation(), FVector::OneVector);

			FMatrix WorldToRoot = RootTransformPivotOnly.ToMatrixWithScale().Inverse();
			InstanceMatrix = InstanceTransform.ToMatrixWithScale() * WorldToRoot;
		}
	}

	for (int32 InstanceIndex = 0; InstanceIndex < NumInstances; InstanceIndex++)
	{
		for (int32 OverallLODIndex = 0; OverallLODIndex < InOverallMaxLODs; OverallLODIndex++)
		{
			int32 LODIndexRead = FMath::Min(OverallLODIndex, NumLODs - 1);

			FRawMesh& RawMesh = OutRawMeshes[OverallLODIndex];
			FRawMeshValidityTracker& RawMeshTracker = OutRawMeshTrackers[OverallLODIndex];
			const FStaticMeshLODResources& LODResource = pInHISMComponent->GetStaticMesh()->GetRenderData()->LODResources[LODIndexRead];
			const int32 BaseVertexIndex = RawMesh.VertexPositions.Num();

			for (int32 VertIndex = 0; VertIndex < LODResource.GetNumVertices(); ++VertIndex)
			{
				RawMesh.VertexPositions.Add(FVector4f(InstanceMatrices[InstanceIndex].TransformPosition((FVector)LODResource.VertexBuffers.PositionVertexBuffer.VertexPosition((uint32)VertIndex))));
			}

			const FIndexArrayView IndexArrayView = LODResource.IndexBuffer.GetArrayView();
			const FStaticMeshVertexBuffer& StaticMeshVertexBuffer = LODResource.VertexBuffers.StaticMeshVertexBuffer;
			const int32 NumTexCoords = FMath::Min(StaticMeshVertexBuffer.GetNumTexCoords(), (uint32)MAX_MESH_TEXTURE_COORDS);
			const int32 NumSections = LODResource.Sections.Num();

			for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
			{
				const FStaticMeshSection& StaticMeshSection = LODResource.Sections[SectionIndex];

				const int32 NumIndices = StaticMeshSection.NumTriangles * 3;
				for (int32 IndexIndex = 0; IndexIndex < NumIndices; IndexIndex++)
				{
					int32 Index = IndexArrayView[StaticMeshSection.FirstIndex + IndexIndex];
					RawMesh.WedgeIndices.Add(BaseVertexIndex + Index);

					RawMesh.WedgeTangentX.Add(FVector4f(InstanceMatrices[InstanceIndex].TransformVector(FVector(StaticMeshVertexBuffer.VertexTangentX(Index)))));
					RawMesh.WedgeTangentY.Add(FVector4f(InstanceMatrices[InstanceIndex].TransformVector(FVector(StaticMeshVertexBuffer.VertexTangentY(Index)))));
					RawMesh.WedgeTangentZ.Add(FVector4f(InstanceMatrices[InstanceIndex].TransformVector(FVector(StaticMeshVertexBuffer.VertexTangentZ(Index)))));

					for (int32 TexCoordIndex = 0; TexCoordIndex < MAX_MESH_TEXTURE_COORDS; TexCoordIndex++)
					{
						if (TexCoordIndex >= NumTexCoords)
						{
							RawMesh.WedgeTexCoords[TexCoordIndex].AddDefaulted();
						}
						else
						{
							RawMesh.WedgeTexCoords[TexCoordIndex].Add(StaticMeshVertexBuffer.GetVertexUV(Index, TexCoordIndex));
							RawMeshTracker.bValidTexCoords[TexCoordIndex] = true;
						}
					}

					if (LODResource.VertexBuffers.ColorVertexBuffer.IsInitialized())
					{
						RawMesh.WedgeColors.Add(LODResource.VertexBuffers.ColorVertexBuffer.VertexColor(Index));
						RawMeshTracker.bValidColors = true;
					}
					else
					{
						RawMesh.WedgeColors.Add(FColor::White);
					}
				}

				// copy face info
				for (uint32 TriIndex = 0; TriIndex < StaticMeshSection.NumTriangles; TriIndex++)
				{
					RawMesh.FaceMaterialIndices.Add(BaseMaterialIndex + StaticMeshSection.MaterialIndex);
					RawMesh.FaceSmoothingMasks.Add(0); // Assume this is ignored as bRecomputeNormals is false
				}
			}
		}
	}

	//ProcessMaterials<UHierarchicalInstancedStaticMeshComponent>(pInHISMComponent, InPackageName, OutMaterials);
}

template <typename ComponentType>
void UMeshManagerSubsystem::ProcessMaterials(ComponentType* pInComponent, const FString& InPackageName, TArray<UMaterialInterface*>& OutMaterials)
{
	const int32 NumMaterials = pInComponent->GetNumMaterials();
	for (int32 MaterialIndex = 0; MaterialIndex < NumMaterials; MaterialIndex++)
	{
		UMaterialInterface* MaterialInterface = pInComponent->GetMaterial(MaterialIndex);
		AddToMaterialList(MaterialInterface, InPackageName, OutMaterials);
	}
}

void UMeshManagerSubsystem::AddToMaterialList(UMaterialInterface* pInMaterialInterface, const FString& InPackageName, TArray<UMaterialInterface*>& OutMaterials)
{
	if (pInMaterialInterface && !pInMaterialInterface->GetOuter()->IsA<UPackage>())
	{
		// Convert runtime material instances to new concrete material instances
		// Create new package
		FString OriginalMaterialName = pInMaterialInterface->GetName();
		FString MaterialPath = FPackageName::GetLongPackagePath(InPackageName) / OriginalMaterialName;
		FString MaterialName;
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		AssetToolsModule.Get().CreateUniqueAssetName(MaterialPath, TEXT(""), MaterialPath, MaterialName);
		UPackage* MaterialPackage = CreatePackage(*MaterialPath);

		// Duplicate the object into the new package
		UMaterialInterface* NewMaterialInterface = DuplicateObject<UMaterialInterface>(pInMaterialInterface, MaterialPackage, *MaterialName);
		NewMaterialInterface->SetFlags(RF_Public | RF_Standalone);

		if (UMaterialInstanceDynamic* MaterialInstanceDynamic = Cast<UMaterialInstanceDynamic>(NewMaterialInterface))
		{
			UMaterialInstanceDynamic* OldMaterialInstanceDynamic = CastChecked<UMaterialInstanceDynamic>(pInMaterialInterface);
			MaterialInstanceDynamic->K2_CopyMaterialInstanceParameters(OldMaterialInstanceDynamic);
		}

		NewMaterialInterface->MarkPackageDirty();

		FAssetRegistryModule::AssetCreated(NewMaterialInterface);

		pInMaterialInterface = NewMaterialInterface;
	}

	OutMaterials.Add(pInMaterialInterface);
}

UStaticMesh* UMeshManagerSubsystem::ConvertMeshComponentsToStaticMesh(const FString& MeshTag, const TArray<UMeshComponent*>& InMeshComponents, const FTransform& InRootTransform /*= FTransform::Identity*/, const FString& InPackageName /*= FString()*/)
{
	UStaticMesh* StaticMesh = nullptr;

	// Build a package name to use
	FString MeshName;
	FString PackageName;
	if (InPackageName.IsEmpty())
	{
		FString NewNameSuggestion = MeshTag;
		FString DefaultPath;
		const FString DefaultDirectory = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::NEW_ASSET);
		FPackageName::TryConvertFilenameToLongPackageName(DefaultDirectory, DefaultPath);

		if (DefaultPath.IsEmpty())
		{
			DefaultPath = TEXT("/Game/Meshes");
		}

		FString PackageNameSuggestion = DefaultPath / NewNameSuggestion;
		FString Name;
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		AssetToolsModule.Get().CreateUniqueAssetName(PackageNameSuggestion, TEXT(""), PackageNameSuggestion, Name);

		TSharedPtr<SDlgPickAssetPath> PickAssetPathWidget;

		PickAssetPathWidget = SNew(SDlgPickAssetPath).Title(LOCTEXT("ConvertToStaticMeshPickName", "Choose A Location To Save The Static Mesh"))
				.DefaultAssetPath(FText::FromString(PackageNameSuggestion));

		if (PickAssetPathWidget->ShowModal() == EAppReturnType::Ok)
		{
			// Get the full name of where we want to create the mesh asset.
			PackageName = PickAssetPathWidget->GetFullAssetPath().ToString();
			MeshName = FPackageName::GetLongPackageAssetName(PackageName);

			// Check if the user inputed a valid asset name, if they did not, give it the generated default name
			if (MeshName.IsEmpty())
			{
				// Use the defaults that were already generated.
				PackageName = PackageNameSuggestion;
				MeshName = *Name;
			}
		}
	}
	else
	{
		PackageName = InPackageName;
		MeshName = *FPackageName::GetLongPackageAssetName(PackageName);
	}

	if (!PackageName.IsEmpty() && !MeshName.IsEmpty())
	{
		TArray<FRawMesh> RawMeshes;
		TArray<UMaterialInterface*> Materials;

		TArray<FRawMeshValidityTracker> RawMeshTrackers;

		FTransform RootTransformPivotOnly(InRootTransform.GetRotation(), InRootTransform.GetLocation(), FVector::OneVector );

		FMatrix WorldToRoot = RootTransformPivotOnly.ToMatrixWithScale().Inverse();

		// first do a pass to determine the max LOD level we will be combining meshes into
		int32 OverallMaxLODs = 0;
		for (UMeshComponent* MeshComponent : InMeshComponents)
		{
			AActor* ComponentOwner = MeshComponent->GetOwner();
			EMeshActorType MeshActorType = GetMeshActorType(ComponentOwner);

			UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent);
			UHierarchicalInstancedStaticMeshComponent* HierarchicalInstancedMeshComponent = Cast<UHierarchicalInstancedStaticMeshComponent>(MeshComponent);

			switch (MeshActorType)
			{
			case EMeshActorType::AutoMapStaticMesh:
				OverallMaxLODs = FMath::Max(StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources.Num(), OverallMaxLODs);
				break;

			case EMeshActorType::AutoMapHierInstancedStaticMesh:
				OverallMaxLODs = FMath::Max(HierarchicalInstancedMeshComponent->GetStaticMesh()->GetRenderData()->LODResources.Num(), OverallMaxLODs);
				break;

			default:
				break;
			}
		}

		// Resize raw meshes to accommodate the number of LODs we will need
		RawMeshes.SetNum(OverallMaxLODs);
		RawMeshTrackers.SetNum(OverallMaxLODs);

		// Export all visible components
		for (UMeshComponent* MeshComponent : InMeshComponents)
		{
			FMatrix ComponentToWorld = MeshComponent->GetComponentTransform().ToMatrixWithScale() * WorldToRoot;

			AActor* ComponentOwner = MeshComponent->GetOwner();
			EMeshActorType MeshActorType = GetMeshActorType(ComponentOwner);

			UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent);
			UHierarchicalInstancedStaticMeshComponent* HierarchicalInstancedMeshComponent = Cast<UHierarchicalInstancedStaticMeshComponent>(MeshComponent);

			switch (MeshActorType)
			{
			case EMeshActorType::AutoMapStaticMesh:
				StaticMeshToRawMeshes(StaticMeshComponent, OverallMaxLODs, ComponentToWorld, PackageName, RawMeshTrackers, RawMeshes, Materials);
				break;

			case EMeshActorType::AutoMapHierInstancedStaticMesh:
				HierarchicalInstancedMeshToRawMeshes(HierarchicalInstancedMeshComponent, OverallMaxLODs, InRootTransform, PackageName, RawMeshTrackers, RawMeshes, Materials);
				break;

			default:
				break;
			}
		}

		uint32 MaxInUseTextureCoordinate = 0;

		// scrub invalid vert color & tex coord data
		check(RawMeshes.Num() == RawMeshTrackers.Num());
		for (int32 RawMeshIndex = 0; RawMeshIndex < RawMeshes.Num(); RawMeshIndex++)
		{
			if (!RawMeshTrackers[RawMeshIndex].bValidColors)
			{
				RawMeshes[RawMeshIndex].WedgeColors.Empty();
			}

			for (uint32 TexCoordIndex = 0; TexCoordIndex < MAX_MESH_TEXTURE_COORDS; TexCoordIndex++)
			{
				if (!RawMeshTrackers[RawMeshIndex].bValidTexCoords[TexCoordIndex])
				{
					RawMeshes[RawMeshIndex].WedgeTexCoords[TexCoordIndex].Empty();
				}
				else
				{
					// Store first texture coordinate index not in use
					MaxInUseTextureCoordinate = FMath::Max(MaxInUseTextureCoordinate, TexCoordIndex);
				}
			}
		}

		// Check if we got some valid data.
		bool bValidData = false;
		for (FRawMesh& RawMesh : RawMeshes)
		{
			if (RawMesh.IsValidOrFixable())
			{
				bValidData = true;
				break;
			}
		}

		if (bValidData)
		{
			// Then find/create it.
			UPackage* Package = CreatePackage(*PackageName);
			check(Package);

			// Create StaticMesh object
			StaticMesh = NewObject<UStaticMesh>(Package, *MeshName, RF_Public | RF_Standalone);
			StaticMesh->InitResources();

			StaticMesh->SetLightingGuid();

			// Determine which texture coordinate map should be used for storing/generating the lightmap UVs
			const uint32 LightMapIndex = FMath::Min(MaxInUseTextureCoordinate + 1, (uint32)MAX_MESH_TEXTURE_COORDS - 1);

			// Add source to new StaticMesh
			for (FRawMesh& RawMesh : RawMeshes)
			{
				if (RawMesh.IsValidOrFixable())
				{
					FStaticMeshSourceModel& SrcModel = StaticMesh->AddSourceModel();
					SrcModel.BuildSettings.bRecomputeNormals = false;
					SrcModel.BuildSettings.bRecomputeTangents = false;
					SrcModel.BuildSettings.bRemoveDegenerates = true;
					SrcModel.BuildSettings.bUseHighPrecisionTangentBasis = false;
					SrcModel.BuildSettings.bUseFullPrecisionUVs = false;
					SrcModel.BuildSettings.bGenerateLightmapUVs = true;
					SrcModel.BuildSettings.SrcLightmapIndex = 0;
					SrcModel.BuildSettings.DstLightmapIndex = LightMapIndex;
					SrcModel.SaveRawMesh(RawMesh);
				}
			}

			// Copy materials to new mesh 
			for (UMaterialInterface* Material : Materials)
			{
				StaticMesh->GetStaticMaterials().Add(FStaticMaterial(Material));
			}

			//Set the Imported version before calling the build
			StaticMesh->SetImportVersion(EImportStaticMeshVersion::LastVersion);

			// Set light map coordinate index to match DstLightmapIndex
			StaticMesh->SetLightMapCoordinateIndex(LightMapIndex);

			// setup section info map
			for (int32 RawMeshLODIndex = 0; RawMeshLODIndex < RawMeshes.Num(); RawMeshLODIndex++)
			{
				const FRawMesh& RawMesh = RawMeshes[RawMeshLODIndex];
				TArray<int32> UniqueMaterialIndices;
				for (int32 MaterialIndex : RawMesh.FaceMaterialIndices)
				{
					UniqueMaterialIndices.AddUnique(MaterialIndex);
				}

				int32 SectionIndex = 0;
				for (int32 UniqueMaterialIndex : UniqueMaterialIndices)
				{
					StaticMesh->GetSectionInfoMap().Set(RawMeshLODIndex, SectionIndex, FMeshSectionInfo(UniqueMaterialIndex));
					SectionIndex++;
				}
			}
			StaticMesh->GetOriginalSectionInfoMap().CopyFrom(StaticMesh->GetSectionInfoMap());

			// Build mesh from source
			StaticMesh->Build(false);
			StaticMesh->PostEditChange();

			StaticMesh->MarkPackageDirty();

			// Notify asset registry of new asset
			FAssetRegistryModule::AssetCreated(StaticMesh);

			// Display notification so users can quickly access the mesh
			if (GIsEditor)
			{
				FNotificationInfo Info(FText::Format(LOCTEXT("SkeletalMeshConverted", "Successfully Converted Mesh"), FText::FromString(StaticMesh->GetName())));
				Info.ExpireDuration = 8.0f;
				Info.bUseLargeFont = false;
				Info.Hyperlink = FSimpleDelegate::CreateLambda([=]() { GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAssets(TArray<UObject*>({ StaticMesh })); });
				Info.HyperlinkText = FText::Format(LOCTEXT("OpenNewAnimationHyperlink", "Open {0}"), FText::FromString(StaticMesh->GetName()));
				TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
				if (Notification.IsValid())
				{
					Notification->SetCompletionState(SNotificationItem::CS_Success);
				}
			}
		}
	}

	return StaticMesh;
}

void UMeshManagerSubsystem::PopulateFilters()
{
	FGameplayTag BaseTerrainMapElementTag = FGameplayTag::RequestGameplayTag(FName(TEXT("AutoMapMeshify.Terrain")));
	FGameplayTag BaseAccentMapElementTag = FGameplayTag::RequestGameplayTag(FName(TEXT("AutoMapMeshify.Accent")));
	FGameplayTag BaseTerrainZoneMapElementTag = FGameplayTag::RequestGameplayTag(FName(TEXT("AutoMapMeshify.Terrain.Zone")));
	FGameplayTag BaseAccentZoneMapElementTag = FGameplayTag::RequestGameplayTag(FName(TEXT("AutoMapMeshify.Accent.Zone")));

	GameplaytagsFilter.Add(BaseTerrainMapElementTag);
	GameplaytagsFilter.Add(BaseAccentMapElementTag);
	GameplaytagsFilter.Add(BaseTerrainZoneMapElementTag);
	GameplaytagsFilter.Add(BaseAccentZoneMapElementTag);
}

bool UMeshManagerSubsystem::IsGameplayTagInFilter(const FGameplayTag& GameplayTagToCheck)
{
	if (GameplayTagToCheck.IsValid())
	{
		return GameplaytagsFilter.Contains(GameplayTagToCheck);
	}
	else
	{
		return false;
	}
}

#undef LOCTEXT_NAMESPACE