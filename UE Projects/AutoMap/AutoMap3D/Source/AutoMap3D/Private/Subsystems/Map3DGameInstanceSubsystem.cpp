#include "Subsystems/Map3DGameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "Actors/MapViewer.h"
#include "Actors/MapArea.h"
#include "Actors/MapMarkerBase.h"
#include "Actors/PlayerMapMarker.h"
#include "Actors/ObjectiveMapMarker.h"
#include "DataAssets/ObjectiveMarkerSettings.h"
#include "DataAssets/MapAreaMaterialSettings.h"
#include "Components/ObjectiveMarkerSpinComponent.h"
#include "Components/ObjectiveMarkerBobComponent.h"
#include "DataAssets/PlayerMarkerSettings.h"
#include "Actors/MapProxyActor.h"
#include "Interfaces/MapProxyInterface.h"
#include "AutoMapSMActor.h"
#include "AutoMapHISMActor.h"
#include "Actors/MapZoneVolume.h"

// Public Functions --------------------------------------------------------------------------------------------

void UMap3DGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitialiseWorldDelegates();
}

void UMap3DGameInstanceSubsystem::Deinitialize()
{
	Super::Deinitialize();
	DeinitialiseWorldDelegates();
	CleanupRegisteredItemsAndParameters();
}

bool UMap3DGameInstanceSubsystem::CloseMap()
{
	TRACE_BOOKMARK(TEXT("Map Is Closed"));
	
	if (RegisteredMapViewer && GetCurrentActiveMapArea())
	{
		RegisteredMapViewer->CloseMap();
		
		for (TSet<AMapArea*>::TIterator It(RegisteredMapAreas); It; ++It)
		{
			AMapArea* MapArea = *It;

			MapArea->CloseMap();
		}

		bIsMapOpen = false;
		return true;
	}

	return false;
}

bool UMap3DGameInstanceSubsystem::OpenMap()
{
	TRACE_BOOKMARK(TEXT("Map Is Opened"))

	if (RegisteredMapViewer && GetCurrentActiveMapArea())
	{
		RegisteredMapViewer->OpenMap();

		for (TSet<AMapArea*>::TIterator It(RegisteredMapAreas); It; ++It)
		{
			AMapArea* MapArea = *It;

			MapArea->OpenMap();
		}

		GetCurrentActiveMapArea()->OpenMap();
		bIsMapOpen = true;
		return true;
	}

	return false;
}

bool UMap3DGameInstanceSubsystem::IsMapOpen() const
{
	return bIsMapOpen;
}

void UMap3DGameInstanceSubsystem::RotateMapView(FVector2f Delta)
{
	if (RegisteredMapViewer)
	{
		RegisteredMapViewer->ApplyRotation(Delta);
	}
}

void UMap3DGameInstanceSubsystem::ZoomMapView(ECameraZoomType ZoomType)
{
	if (RegisteredMapViewer)
	{
		RegisteredMapViewer->ApplyZoom(ZoomType);
	}
}

void UMap3DGameInstanceSubsystem::PanMapViewXY(FVector2f Delta)
{
	if (RegisteredMapViewer)
	{
		RegisteredMapViewer->ApplyPanXY(Delta);
	}
}

void UMap3DGameInstanceSubsystem::PanMapViewUp()
{
	if (RegisteredMapViewer)
	{
		RegisteredMapViewer->ApplyPanUp();
	}
}

void UMap3DGameInstanceSubsystem::PanMapViewDown()
{
	if (RegisteredMapViewer)
	{
		RegisteredMapViewer->ApplyPanDown();
	}
}

void UMap3DGameInstanceSubsystem::FocusPlayerMarker()
{
	if (RegisteredMapViewer)
	{
		RegisteredMapViewer->FocusPlayer();
	}
}

void UMap3DGameInstanceSubsystem::PaintMapArea(const FVector& PaintLocation, float InTerrainPaintRadius, float InAccentPaintRadius)
{
	if (GetCurrentActiveMapArea())
	{
		GetCurrentActiveMapArea()->PaintArea(PaintLocation, InTerrainPaintRadius);
	}
}

void UMap3DGameInstanceSubsystem::CalculateMapWorldPosition(const FVector& InWorldPosition, FVector& OutMapPosition)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UMap3DGameInstanceSubsystem::CalculateMapWorldPosition)

	FVector LocalPos{};

	// Calculate local position of actor relative to the terrain auto map actors root transform
	CalculateMapLocalPosition(InWorldPosition, LocalPos);

	if(GetCurrentActiveMapArea())
	{
		// Calculate new world position relative to the map area
		OutMapPosition = GetCurrentActiveMapArea()->GetActorLocation() + LocalPos;
	}
}

void UMap3DGameInstanceSubsystem::GetPlayerMarkerPosition(FVector& OutPlayerMarkerPosition)
{
	if (RegisteredPlayerMarker)
	{
		OutPlayerMarkerPosition = RegisteredPlayerMarker->GetActorLocation();
	}
	else
	{
		OutPlayerMarkerPosition = FVector::Zero();
	}
}

void UMap3DGameInstanceSubsystem::GetPlayerMarkerRotation(FRotator& OutPlayerMarkerRotation)
{
	if (RegisteredPlayerMarker)
	{
		OutPlayerMarkerRotation = RegisteredPlayerMarker->GetActorRotation();
	}
	else
	{
		OutPlayerMarkerRotation = FRotator::ZeroRotator;
	}
}

void UMap3DGameInstanceSubsystem::GetMapViewerPosition(FVector& OutMapViewerPosition)
{
	if (RegisteredMapViewer)
	{
		OutMapViewerPosition = RegisteredMapViewer->GetActorLocation();
	}
	else
	{
		OutMapViewerPosition = FVector::Zero();
	}
}

void UMap3DGameInstanceSubsystem::GetMapViewerForwardDir(FVector& OutMapViewerForward)
{
	if (RegisteredMapViewer)
	{
		OutMapViewerForward = RegisteredMapViewer->GetActorForwardVector();
	}
	else
	{
		OutMapViewerForward = FVector::Zero();
	}
}

const AActor* UMap3DGameInstanceSubsystem::GetProxyParent(AMapProxyActor* InMapProxyActor)
{
	if (!MapProxyActorsMap.IsEmpty() && InMapProxyActor)
	{
		FGameplayTag MapProxyZoneTag;

		InMapProxyActor->GetZoneTag(MapProxyZoneTag);

		if(MapProxyActorsMap.Contains(MapProxyZoneTag))
		{
			const FMapProxyZoneContainer& MapProxyZoneContainer = *MapProxyActorsMap.Find(MapProxyZoneTag);

			if (MapProxyZoneContainer.MapProxyActorsToParentActors.Contains(InMapProxyActor))
			{
				return *MapProxyZoneContainer.MapProxyActorsToParentActors.Find(InMapProxyActor);
			}
			else
			{
				return nullptr;
			}
		}
		else
		{
			return nullptr;
		}
	}

	return nullptr;
}

FObjectiveMapMarkerSpawnPair UMap3DGameInstanceSubsystem::SpawnObjectiveMarker(const FVector& InSpawnLocation, EObjectiveMapMarkerType InObjectiveMarkerType)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UMap3DGameInstanceSubsystem::SpawnObjectiveMarker)

	UWorld* World = GetWorld();

	if (World)
	{
		FActorSpawnParameters SpawnInfo;
		AObjectiveMapMarker* SpawnedMapWorldObjectiveMarker{nullptr};
		AObjectiveMapMarker* SpawnedGameWorldObjectiveMarker{nullptr};
		
		FVector TransformedSpawnLocation;
		CalculateMapWorldPosition(InSpawnLocation, TransformedSpawnLocation);
				
		// Spawn objective marker with transformed position
		SpawnedMapWorldObjectiveMarker = World->SpawnActor<AObjectiveMapMarker>(AObjectiveMapMarker::StaticClass(), TransformedSpawnLocation, FRotator(), SpawnInfo);
		
		// Spawn objective marker at non transformed position
		SpawnedGameWorldObjectiveMarker = World->SpawnActor<AObjectiveMapMarker>(AObjectiveMapMarker::StaticClass(), InSpawnLocation, FRotator(), SpawnInfo);
		
		// If we have successfully spawned in the object marker in both the game world and map world and we have settings to fill out objective marker parameters
		if (SpawnedMapWorldObjectiveMarker && SpawnedGameWorldObjectiveMarker && ObjectiveMarkerSettings)
		{
			bool bSuccessfullyAddedMapWorldObjectiveMarkerMesh = SetObjectiveMarkerMesh(SpawnedMapWorldObjectiveMarker, GetObjectiveMarkerMesh(InObjectiveMarkerType));

			SpawnedGameWorldObjectiveMarker->SetIsObjectiveMarkerInMapWorld(false);
			bool bSuccessfullyAddedGameWorldObjectiveMarkerMesh = SetObjectiveMarkerMesh(SpawnedGameWorldObjectiveMarker, GetObjectiveMarkerMesh(InObjectiveMarkerType));

			if (bSuccessfullyAddedMapWorldObjectiveMarkerMesh && bSuccessfullyAddedGameWorldObjectiveMarkerMesh)
			{
				// Setting map world objective marker parameters
				SpawnedMapWorldObjectiveMarker->SetMarkerDeletionType(GetObjectiveMarkerDeletionSetting(InObjectiveMarkerType));
				AttachObjectiveMarkerBehaviourComponents(SpawnedMapWorldObjectiveMarker, InObjectiveMarkerType);
				
				// Setting game world objective marker parameters
				SpawnedGameWorldObjectiveMarker->SetMarkerDeletionType(GetObjectiveMarkerDeletionSetting(InObjectiveMarkerType));
				AttachObjectiveMarkerBehaviourComponents(SpawnedGameWorldObjectiveMarker, InObjectiveMarkerType);
				
				AddObjectToMapView(SpawnedMapWorldObjectiveMarker);
			}
		}

		// If we have successfully spawned in both object markers
		if (SpawnedMapWorldObjectiveMarker && SpawnedGameWorldObjectiveMarker)
		{
			RegisterObjectiveMapMarker(SpawnedMapWorldObjectiveMarker, SpawnedGameWorldObjectiveMarker);

			FObjectiveMapMarkerSpawnPair ObjectiveMapMarkerSpawnPair;

			ObjectiveMapMarkerSpawnPair.MapWorldObjectiveMarker = SpawnedMapWorldObjectiveMarker;
			ObjectiveMapMarkerSpawnPair.GameWorldObjectiveMarker = SpawnedGameWorldObjectiveMarker;

			return ObjectiveMapMarkerSpawnPair;
		}
		else
		{
			return FObjectiveMapMarkerSpawnPair();
		}
	}

	return FObjectiveMapMarkerSpawnPair();
}

void UMap3DGameInstanceSubsystem::RemoveObjectFromMapView(AActor* InActor)
{
	if (RegisteredMapViewer && InActor)
	{
		RegisteredMapViewer->RemoveObjectFromMapView(InActor);

		AMapProxyActor* MapProxy = Cast<AMapProxyActor>(InActor);

		if (MapProxy)
		{
			FGameplayTag MapProxyZoneTag;

			MapProxy->GetZoneTag(MapProxyZoneTag);

			if (MapProxyActorsMap.Contains(MapProxyZoneTag))
			{
				FMapProxyZoneContainer& MapProxyZoneContainer = *MapProxyActorsMap.Find(MapProxyZoneTag);
				MapProxyZoneContainer.MapProxyActorsToParentActors.Remove(MapProxy);
			}
		}
			
		InActor->Destroy();
	}
}

void UMap3DGameInstanceSubsystem::RemoveObjectiveMarker(AActor* InActor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UMap3DGameInstanceSubsystem::RemoveObjectiveMarker)

	if (RegisteredMapViewer && InActor)
	{
		RegisteredMapViewer->RemoveObjectFromMapView(InActor);

		AObjectiveMapMarker* MapWorldObjectiveMarker = Cast<AObjectiveMapMarker>(InActor);

		if (MapWorldObjectiveMarker)
		{
			AObjectiveMapMarker* GameWorldObjectiveMarker = *RegisteredObjectiveMapMarkers.Find(MapWorldObjectiveMarker);

			if(GameWorldObjectiveMarker)
			{
				GameWorldObjectiveMarker->Destroy();
			}

			RegisteredObjectiveMapMarkers.Remove(MapWorldObjectiveMarker);
		}

		InActor->Destroy();
	}
}

float UMap3DGameInstanceSubsystem::GetObjectiveMarkerDeleteDistance()
{
	if(ObjectiveMarkerSettings)
	{
		return ObjectiveMarkerSettings->GetObjectiveMarkerDeletionDistance();
	}
	else
	{
		return 50.0f;
	}
}

void UMap3DGameInstanceSubsystem::GetMapViewerFOV(float& OutFOV)
{
	if (RegisteredMapViewer)
	{
		OutFOV = RegisteredMapViewer->GetFOV();
	}
	else
	{
		OutFOV = 0.0f;
	}
}

void UMap3DGameInstanceSubsystem::GetMapViewerMaxFOV(float& OutFOV)
{
	if (RegisteredMapViewer)
	{
		OutFOV = RegisteredMapViewer->GetMaxFOV();
	}
	else
	{
		OutFOV = 0.0f;
	}
}

void UMap3DGameInstanceSubsystem::SetMapViewerFOV(float InFOV)
{
	if (RegisteredMapViewer)
	{
		RegisteredMapViewer->SetFOV(InFOV);
	}
}

void UMap3DGameInstanceSubsystem::SetMapViewerFOVUsingAlpha(float InAlpha)
{
	if(RegisteredMapViewer)
	{
		float MaxFOV = RegisteredMapViewer->GetMaxFOV();
		float NewFOV = MaxFOV * InAlpha;

		RegisteredMapViewer->SetFOV(NewFOV);
	}
}

void UMap3DGameInstanceSubsystem::GetMapViewerPlayerFollowBackwardsOffset(float& OutOffset)
{
	if (RegisteredMapViewer)
	{
		OutOffset = RegisteredMapViewer->GetPlayerFollowBackwardsOffset();
	}
	else
	{
		OutOffset = 0.0f;
	}
}

void UMap3DGameInstanceSubsystem::GetMapViewerPlayerFollowUpwardsOffset(float& OutOffset)
{
	if (RegisteredMapViewer)
	{
		OutOffset = RegisteredMapViewer->GetPlayerFollowUpwardsOffset();
	}
	else
	{
		OutOffset = 0.0f;
	}
}

float UMap3DGameInstanceSubsystem::GetMaxMapViewerPlayerFollowDistance()
{
	return 10000.0f;
}

void UMap3DGameInstanceSubsystem::SetMapViewerPlayerFollowBackwardsOffset(float InOffset)
{
	if (RegisteredMapViewer)
	{
		RegisteredMapViewer->SetPlayerFollowBackwardsOffset(InOffset);
	}
}

void UMap3DGameInstanceSubsystem::SetMapViewerPlayerFollowUpwardsOffset(float InOffset)
{
	if (RegisteredMapViewer)
	{
		RegisteredMapViewer->SetPlayerFollowUpwardsOffset(InOffset);
	}
}

void UMap3DGameInstanceSubsystem::SetMapViewerPlayerFollowBackwardsOffsetUsingAlpha(float InAlpha)
{
	if (RegisteredMapViewer)
	{
		float MaxPlayerFollowDistance = GetMaxMapViewerPlayerFollowDistance();
		float NewPlayerFollowDistanceY = MaxPlayerFollowDistance * InAlpha;

		RegisteredMapViewer->SetPlayerFollowBackwardsOffset(NewPlayerFollowDistanceY);
	}
}

void UMap3DGameInstanceSubsystem::SetMapViewerPlayerFollowUpOffsetUsingAlpha(float InAlpha)
{
	if (RegisteredMapViewer)
	{
		float MaxPlayerFollowDistance = GetMaxMapViewerPlayerFollowDistance();
		float NewPlayerFollowDistanceZ = MaxPlayerFollowDistance * InAlpha;

		RegisteredMapViewer->SetPlayerFollowUpwardsOffset(NewPlayerFollowDistanceZ);
	}
}

void UMap3DGameInstanceSubsystem::GetMapZoomOffset(float& OutOffset)
{
	if (RegisteredMapViewer)
	{
		OutOffset = RegisteredMapViewer->GetZoomDistanceOffset();
	}else
	{
		OutOffset = 0.0f;
	}
}

float UMap3DGameInstanceSubsystem::GetMaxMapZoomOffset()
{
	return 10000.0f;
}

void UMap3DGameInstanceSubsystem::SetMapZoomOffset(float InOffset)
{
	if (RegisteredMapViewer)
	{
		RegisteredMapViewer->SetZoomDistanceOffset(InOffset);
	}
}

void UMap3DGameInstanceSubsystem::SetMapZoomOffsetUsingAlpha(float InAlpha)
{
	if (RegisteredMapViewer)
	{
		float MaxZoomOffset = GetMaxMapZoomOffset();
		float NewZoomOffset = MaxZoomOffset * InAlpha;

		RegisteredMapViewer->SetZoomDistanceOffset(NewZoomOffset);
	}
}

void UMap3DGameInstanceSubsystem::ResizeViewerRenderTarget(int32 InWidth, int32 InHeight)
{
	if(RegisteredMapViewer)
	{
		RegisteredMapViewer->ResizeViewerRenderTarget(InWidth, InHeight);
	}
}

void UMap3DGameInstanceSubsystem::ActivateMapArea(AMapArea* InInactiveMapArea)
{
	if (InInactiveMapArea)
	{
		CurrentActiveMapAreaIndex = RegisteredMapAreas.FindId(InInactiveMapArea);

		if(!InInactiveMapArea->IsMapAreaDiscovered())
		{
			AddComponentsToMapView(InInactiveMapArea->GetTerrainMeshComponent());
			AddComponentsToMapView(InInactiveMapArea->GetAccentMeshComponent());

			FGameplayTag ZoneTag;
			InInactiveMapArea->GetZoneTag(ZoneTag);
			
			AddMapProxiesToView(ZoneTag);

			InInactiveMapArea->DiscoverMapArea();
		}
	}
}

void UMap3DGameInstanceSubsystem::ActivateMapArea(const FGameplayTag InMapZoneTagToActivate)
{
	if (InMapZoneTagToActivate.IsValid() && MapZoneTagToMapAreaSetIndexMap.Contains(InMapZoneTagToActivate))
	{
		const FSetElementId& MapAreaSetIndex = *MapZoneTagToMapAreaSetIndexMap.Find(InMapZoneTagToActivate);

		ActivateMapArea(MapAreaSetIndex);
	}
}

bool UMap3DGameInstanceSubsystem::IsMapAreaActive(const AMapArea* InMapAreaToCheck)
{
	if(!RegisteredMapAreas.IsEmpty())
	{
		FSetElementId InMapAreaToCheckSetIndex = RegisteredMapAreas.FindId(InMapAreaToCheck);

		if(InMapAreaToCheckSetIndex.IsValidId())
		{
			if(CurrentActiveMapAreaIndex == InMapAreaToCheckSetIndex)
			{
				return true;
			}
			else
			{
				return false;
			}

		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool UMap3DGameInstanceSubsystem::IsMapAreaActive(const FGameplayTag InZoneTag)
{
	if(InZoneTag.IsValid() && MapZoneTagToMapAreaSetIndexMap.Contains(InZoneTag))
	{
		const FSetElementId RequestMapAreaIndex = *MapZoneTagToMapAreaSetIndexMap.Find(InZoneTag);

		if(RequestMapAreaIndex.IsValidId() && RequestMapAreaIndex == CurrentActiveMapAreaIndex)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

void UMap3DGameInstanceSubsystem::RegisterMapViewer(AMapViewer* MapViewer)
{	
	if(MapViewer)
	{
		RegisteredMapViewer = MapViewer;
	}
}

void UMap3DGameInstanceSubsystem::RegisterMapArea(AMapArea* MapArea)
{
	if (MapArea)
	{
		RegisteredMapAreas.Add(MapArea);
	}
}

void UMap3DGameInstanceSubsystem::RegisterPlayerMapMarker(AMapMarkerBase* InMapMarker)
{
	if (InMapMarker)
	{
		APlayerMapMarker* PlayerMarker = Cast<APlayerMapMarker>(InMapMarker);

		if (PlayerMarker)
		{
			RegisteredPlayerMarker = InMapMarker;
		}
	}
}

void UMap3DGameInstanceSubsystem::RegisterObjectiveMapMarker(AObjectiveMapMarker* InMapWorldObjectiveMarker, AObjectiveMapMarker* InGameWorldObjectiveMarker)
{
	if(InMapWorldObjectiveMarker && InGameWorldObjectiveMarker)
	{
		RegisteredObjectiveMapMarkers.Add(InMapWorldObjectiveMarker, InGameWorldObjectiveMarker);
	}
}

void UMap3DGameInstanceSubsystem::RegisterMapZoneVolume(AMapZoneVolume* InMapZoneVolume)
{
	RegisteredMapZoneVolumes.AddUnique(InMapZoneVolume);
}

void UMap3DGameInstanceSubsystem::CalculateMapLocalPosition(const FVector& InWorldPosition, FVector& OutLocalPosition)
{
	OutLocalPosition = GetCurrentActiveMapAreaInverseTransform().TransformPosition(InWorldPosition);
}

void UMap3DGameInstanceSubsystem::CalculateMapEnvironmentTransform(AMapArea* InMapArea, int32 InTransformIndex)
{	
	if (!InMapArea)
	{
		MapAreaRootInverseTransforms[InTransformIndex] = FTransform::Identity;
		return;
	}

	FVector BakedOrigin = InMapArea->GetGameWorldOrigin();

	if (BakedOrigin.IsNearlyZero())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Map3D: MapArea '%s' has a zero GameWorldOrigin — did you run BakeGameWorldOrigin in the editor?"),
			*InMapArea->GetName());
	}

	FTransform OriginTransform(FRotator::ZeroRotator, BakedOrigin, FVector::OneVector);
	MapAreaRootInverseTransforms[InTransformIndex] = OriginTransform.Inverse();
}

void UMap3DGameInstanceSubsystem::RegisterStaticMapElements()
{
	if (GEngine)
	{
		// Registering Map Viewer
		{
			AActor* Actor = UGameplayStatics::GetActorOfClass(GetWorld(), AMapViewer::StaticClass());

			if (Actor)
			{
				AMapViewer* MapViewer = Cast<AMapViewer>(Actor);

				if (MapViewer)
				{
					RegisterMapViewer(MapViewer);
				}
			}
		}

		//Registering Map Areas
		{
			TArray<AActor*> UnregisteredMapAreas{};

			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMapArea::StaticClass(), UnregisteredMapAreas);
			
			for(AActor* Actor : UnregisteredMapAreas)
			{
				if (Actor)
				{
					AMapArea* MapArea = Cast<AMapArea>(Actor);

					if (MapArea)
					{
						if (MapArea->IsUsingHighResolutionRT())
						{
							MapArea->SetTerrainPaintTextureTarget(CreatePaintRenderTarget(512, 512));
						}
						else
						{
							MapArea->SetTerrainPaintTextureTarget(CreatePaintRenderTarget(256, 256));
						}

						MapArea->SetupPreBeginPlayParameters();

						RegisterMapArea(MapArea);
					}
				}
			}

			TArray<AMapArea*> NewlyRegisteredMapAreas = RegisteredMapAreas.Array();
			MapAreaRootInverseTransforms.Empty();

			MapAreaRootInverseTransforms.SetNum(NewlyRegisteredMapAreas.Num());

			for (AMapArea* MapArea : NewlyRegisteredMapAreas)
			{
				FSetElementId MapAreaSetIndex = RegisteredMapAreas.FindId(MapArea);
				int32 MapAreaArrayIndex  = MapAreaSetIndex.AsInteger();
				
				FGameplayTag MapAreaZoneTag;
				MapArea->GetZoneTag(MapAreaZoneTag);
				 
				AddMapProxyZoneKey(MapAreaZoneTag);

				AddMapZoneVolumeZoneKey(MapAreaZoneTag, MapAreaSetIndex);

				FGameplayTag StartingMapAreaZoneTag = FGameplayTag::RequestGameplayTag(FName(TEXT("AutoMapGameplay.MapZone_1")));

				if (MapAreaZoneTag.MatchesTagExact(StartingMapAreaZoneTag))
				{
					CurrentActiveMapAreaIndex = MapAreaSetIndex;
				}

				// Populating array of inverse transforms per zone
				CalculateMapEnvironmentTransform(MapArea, MapAreaArrayIndex);
			}
		}
	}
}

void UMap3DGameInstanceSubsystem::GetMapElementsWithZoneTag(TArray<AActor*>& TerrainActors, TArray<AActor*>& AccentActors, const FGameplayTag& InMapAreaZoneTag)
{
	TArray<AActor*> MapEnvironmentActors{};

	FName MapZoneTagName = InMapAreaZoneTag.GetTagName();
	
	FName MapZoneTerrainTagName = *(InMapAreaZoneTag.GetTagName().ToString() + FString(TEXT(".Terrain")));
	FName MapZoneAccentTagName = *(InMapAreaZoneTag.GetTagName().ToString() + FString(TEXT(".Accent")));

	const FGameplayTag TerrainGameplayTag = FGameplayTag::RequestGameplayTag(MapZoneTerrainTagName);
	const FGameplayTag AccentGameplayTag = FGameplayTag::RequestGameplayTag(MapZoneAccentTagName);

	if (GEngine)
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAutoMapSMActor::StaticClass(), MapEnvironmentActors);

		for (AActor* WorldActor : MapEnvironmentActors)
		{
			if (WorldActor)
			{
				AAutoMapSMActor* AutoMapStaticMeshActor = Cast<AAutoMapSMActor>(WorldActor);

				if (AutoMapStaticMeshActor)
				{
					FGameplayTagContainer EnvironmentActorTagContainer{};

					const FGameplayTag ZoneTag = AutoMapStaticMeshActor->GetMapZoneTag();

					if (ZoneTag.MatchesTag(TerrainGameplayTag))
					{
						TerrainActors.AddUnique(WorldActor);
					}
					else if (ZoneTag.MatchesTag(AccentGameplayTag))
					{
						AccentActors.AddUnique(WorldActor);
					}
				}
			}
		}

		MapEnvironmentActors.Empty();

		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAutoMapHISMActor::StaticClass(), MapEnvironmentActors);

		for (AActor* WorldActor : MapEnvironmentActors)
		{
			if (WorldActor)
			{
				AAutoMapHISMActor* HISMAutoMapActor = Cast<AAutoMapHISMActor>(WorldActor);

				if (HISMAutoMapActor)
				{
					FGameplayTagContainer EnvironmentActorTagContainer{};

					const FGameplayTag ZoneTag = HISMAutoMapActor->GetMapZoneTag();

					if (ZoneTag.MatchesTag(TerrainGameplayTag))
					{
						TerrainActors.AddUnique(WorldActor);
					}
					else if (ZoneTag.MatchesTag(AccentGameplayTag))
					{
						AccentActors.AddUnique(WorldActor);
					}
				}
			}
		}
	}
}

void UMap3DGameInstanceSubsystem::AddObjectToMapView(AActor* InActor)
{
	if (RegisteredMapViewer)
	{
		if (InActor)
		{
			RegisteredMapViewer->AddObjectToMapView(InActor);
		}
	}
}

void UMap3DGameInstanceSubsystem::AddComponentsToMapView(UPrimitiveComponent* InComponent)
{
	if (RegisteredMapViewer)
	{
		if (InComponent)
		{
			RegisteredMapViewer->AddComponentToMapView(InComponent);
		}
	}
}

void UMap3DGameInstanceSubsystem::SetObjectiveMarkerSettings(UObjectiveMarkerSettings* InSettings)
{
	if (InSettings)
	{
		ObjectiveMarkerSettings = InSettings;
	}
}

void UMap3DGameInstanceSubsystem::SetMapAreaMaterialSettings(UMapAreaMaterialSettings* InSettings)
{
	if (InSettings)
	{
		MapAreaMaterialSettings = InSettings;
	}
}

void UMap3DGameInstanceSubsystem::SetPlayerMarkerSettings(UPlayerMarkerSettings* InSettings)
{
	if (InSettings)
	{
		PlayerMapMarkerSettings = InSettings;
	}
}

void UMap3DGameInstanceSubsystem::SetUpMap(UObjectiveMarkerSettings* InObjectiveMarkerSettings, UMapAreaMaterialSettings* InMapAreaMaterialSettings, UPlayerMarkerSettings* InPlayerMarkerSettings)
{
	SetObjectiveMarkerSettings(InObjectiveMarkerSettings);
	SetMapAreaMaterialSettings(InMapAreaMaterialSettings);
	SetPlayerMarkerSettings(InPlayerMarkerSettings);

	SetupPlayerMarker();
	SetupMapAreaMaterials();

	SpawnMapProxies(GetWorld());

	ActivateMapArea(CurrentActiveMapAreaIndex);
}

void UMap3DGameInstanceSubsystem::SetupMapAreaMaterials()
{
	if(MapAreaMaterialSettings && !RegisteredMapAreas.IsEmpty())
	{
		TArray<AMapArea*> MapAreas = RegisteredMapAreas.Array();

		for(AMapArea* MapArea: MapAreas)
		{
			if(MapArea)
			{
				MapArea->SetDynamicTerrainUnwrapMaterial(CreateDynamicTerrainUnwrapMaterial());
				MapArea->SetDynamicTerrainMeshMaterial(CreateDynamicTerrainMeshMaterial());
				MapArea->SetDynamicAccentMeshMaterial(CreateDynamicAccentMeshMaterial());
				MapArea->SetupMaterialsParameters();
			}
		}
	}
}

void UMap3DGameInstanceSubsystem::SetupPlayerMarker()
{
	APlayerMapMarker* SpawnedPlayerMapMarker = SpawnPlayerMapMarker();

	if(SpawnedPlayerMapMarker && PlayerMapMarkerSettings)
	{
		SpawnedPlayerMapMarker->SetMesh(PlayerMapMarkerSettings->GetPlayerMapMarkerMesh());
		AddObjectToMapView(SpawnedPlayerMapMarker);
		RegisterPlayerMapMarker(SpawnedPlayerMapMarker);
	}
}

APlayerMapMarker* UMap3DGameInstanceSubsystem::SpawnPlayerMapMarker()
{
	FActorSpawnParameters SpawnInfo;

	APlayerMapMarker* SpawnedPlayerMapMarker = GetWorld()->SpawnActor<APlayerMapMarker>(APlayerMapMarker::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);

	if(SpawnedPlayerMapMarker)
	{
		return SpawnedPlayerMapMarker;
	}
	else
	{
		return nullptr;
	}
}

UMaterialInstanceDynamic* UMap3DGameInstanceSubsystem::CreateDynamicTerrainUnwrapMaterial()
{
	if(MapAreaMaterialSettings)
	{
		return UMaterialInstanceDynamic::Create(MapAreaMaterialSettings->GetUnwrapMaterial(), this);
	}

	return nullptr;
}

UMaterialInstanceDynamic* UMap3DGameInstanceSubsystem::CreateDynamicTerrainMeshMaterial()
{
	if (MapAreaMaterialSettings)
	{
		return UMaterialInstanceDynamic::Create(MapAreaMaterialSettings->GetTerrainMaterial(), this);
	}

	return nullptr;
}

UMaterialInstanceDynamic* UMap3DGameInstanceSubsystem::CreateDynamicAccentMeshMaterial()
{
	if (MapAreaMaterialSettings)
	{
		return UMaterialInstanceDynamic::Create(MapAreaMaterialSettings->GetAccentMaterial(), this);
	}

	return nullptr;
}

UTextureRenderTarget2D* UMap3DGameInstanceSubsystem::CreatePaintRenderTarget(int32 InRenderTargetWidth, int32 InRenderTargetHeight)
{
	UTextureRenderTarget2D* RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, InRenderTargetWidth, InRenderTargetHeight);
	
	RenderTarget->AddressX = TextureAddress::TA_Clamp;
	RenderTarget->AddressY = TextureAddress::TA_Clamp;

	RenderTarget->UpdateResourceImmediate(true);

	return RenderTarget;
}

bool UMap3DGameInstanceSubsystem::SetObjectiveMarkerMesh(AObjectiveMapMarker* InObjectiveMarker, UStaticMesh* InStaticMesh)
{
	if(InObjectiveMarker && InStaticMesh)
	{
		return InObjectiveMarker->SetMesh(InStaticMesh);
	}

	return false;
}

UStaticMesh* UMap3DGameInstanceSubsystem::GetObjectiveMarkerMesh(EObjectiveMapMarkerType InObjectiveMarkerType)
{
	if(ObjectiveMarkerSettings)
	{
		return ObjectiveMarkerSettings->GetObjectiveMarkerMesh(InObjectiveMarkerType);
	}
	else
	{
		return nullptr;
	}
}

EObjectiveMapMarkerDeletionType UMap3DGameInstanceSubsystem::GetObjectiveMarkerDeletionSetting(EObjectiveMapMarkerType InObjectiveMarkerType)
{
	if (ObjectiveMarkerSettings)
	{
		return ObjectiveMarkerSettings->GetObjectiveMarkerDeletionType(InObjectiveMarkerType);
	}
	else
	{
		return EObjectiveMapMarkerDeletionType::Del_Null;
	}
}

void UMap3DGameInstanceSubsystem::AttachObjectiveMarkerBehaviourComponents(AObjectiveMapMarker* InObjectiveMarker, EObjectiveMapMarkerType InObjectiveMarkerType)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UMap3DGameInstanceSubsystem::AttachObjectiveMarkerBehaviourComponents)

	if (ObjectiveMarkerSettings && InObjectiveMarker)
	{
		FObjectiveMarkerBehaviourSettings ObjectiveMarkerBehaviourSettings;
		
		ObjectiveMarkerSettings->GetObjectiveMarkerBehaviourSettings(InObjectiveMarkerType, ObjectiveMarkerBehaviourSettings);

		for(const EObjectiveMapMarkerBehaviour& Behaviour : ObjectiveMarkerBehaviourSettings.BehaviourStack)
		{
			switch (Behaviour)
			{
				case EObjectiveMapMarkerBehaviour::Be_Rotate:
				{
					UObjectiveMarkerSpinComponent* SpinComponent = Cast<UObjectiveMarkerSpinComponent>(InObjectiveMarker->AddComponentByClass(UObjectiveMarkerSpinComponent::StaticClass(), false, FTransform::Identity, false));
					
					if(SpinComponent)
					{
						SpinComponent->SetRotationSpeed(ObjectiveMarkerBehaviourSettings.RotationSpeed);
					}

					break;
				}

				case EObjectiveMapMarkerBehaviour::Be_Bob:
				{
					UObjectiveMarkerBobComponent* BobComponent = Cast<UObjectiveMarkerBobComponent>(InObjectiveMarker->AddComponentByClass(UObjectiveMarkerBobComponent::StaticClass(), false, FTransform::Identity, false));

					if (BobComponent)
					{
						BobComponent->SetBobLength(ObjectiveMarkerBehaviourSettings.BobLength);
						BobComponent->SetBobSpeed(ObjectiveMarkerBehaviourSettings.BobSpeed);
						BobComponent->SetOwnerSpawnPosition(InObjectiveMarker->GetActorLocation());
					}

					break;
				}

				default:
				{
					break;
				}
			}
		}
	}
}

void UMap3DGameInstanceSubsystem::RegisterMapProxyActor(FGameplayTag InZoneTag, AMapProxyActor* MapProxy, AActor* ParentActor)
{
	if(MapProxy && ParentActor)
	{
		if(MapProxyActorsMap.Contains(InZoneTag))
		{
			FMapProxyZoneContainer& MapProxyZoneContainer = *MapProxyActorsMap.Find(InZoneTag);
			
			MapProxyZoneContainer.MapProxyActorsToParentActors.Add(MapProxy, ParentActor);
		}
	}
}

void UMap3DGameInstanceSubsystem::SpawnMapProxies(UWorld* InWorld)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UMap3DGameInstanceSubsystem::SpawnMapProxies)

	TArray<AActor*> ActorsWithInterface{};

	UGameplayStatics::GetAllActorsWithInterface(InWorld, UMapProxyInterface::StaticClass(), ActorsWithInterface);

	for (AActor* Actor : ActorsWithInterface)
	{
		FVector GameWorldPosition = IMapProxyInterface::Execute_GetGameWorldPosition(Actor);
		FRotator ActorRotation = Actor->GetActorRotation();
		UStaticMesh* StaticMeshToCopy = IMapProxyInterface::Execute_GetMapProxyMesh(Actor);
		bool bShouldUpdateTransform = IMapProxyInterface::Execute_ShouldUpdateTransform(Actor);
		FGameplayTag ZoneTag = IMapProxyInterface::Execute_GetMapZoneGamePlayTag(Actor);

		FVector SpawnPosition;
		CalculateMapWorldPosition(GameWorldPosition, SpawnPosition);
			
		// If we have a static mesh to copy
		if (StaticMeshToCopy)
		{
			AMapProxyActor* SpawnedProxy = SpawnMapProxy(InWorld, ZoneTag, SpawnPosition, ActorRotation, StaticMeshToCopy, bShouldUpdateTransform);

			if(SpawnedProxy)
			{
				RegisterMapProxyActor(ZoneTag, SpawnedProxy, Actor);
			}
		}
	}
}

AMapProxyActor* UMap3DGameInstanceSubsystem::SpawnMapProxy(UWorld* InWorld, const FGameplayTag& InZoneTag, const FVector& InSpawnLocation, const  FRotator& InSpawnRotation, UStaticMesh* InMeshToCopy, bool bShouldUpdateTransform)
{
	FActorSpawnParameters SpawnInfo;

	AMapProxyActor* SpawnedProxy = InWorld->SpawnActor<AMapProxyActor>(AMapProxyActor::StaticClass(), InSpawnLocation, InSpawnRotation, SpawnInfo);

	if(SpawnedProxy)
	{
		SpawnedProxy->SetMapProxyMesh(InMeshToCopy);

		if(MapAreaMaterialSettings)
		{
			SpawnedProxy->SetMapProxyMaterial(MapAreaMaterialSettings->GetMapProxyMaterial());
		}

		SpawnedProxy->SetZoneTag(InZoneTag);
		SpawnedProxy->SetShouldUpdateTransform(bShouldUpdateTransform);
		return SpawnedProxy;
	}

	return nullptr;
}

void UMap3DGameInstanceSubsystem::AddMapProxyZoneKey(const FGameplayTag& InZoneTag)
{
	MapProxyActorsMap.Add({InZoneTag});
}

void UMap3DGameInstanceSubsystem::AddMapProxiesToView(const FGameplayTag& InZoneTag)
{
	const FMapProxyZoneContainer& MapProxyZoneContainer = *MapProxyActorsMap.Find(InZoneTag);
	
	for (const TPair<AMapProxyActor*, AActor*>& Pair : MapProxyZoneContainer.MapProxyActorsToParentActors)
	{
		AMapProxyActor* MapProxy = Pair.Key;

		if(MapProxy)
		{
			AddObjectToMapView(MapProxy);
		}
	}
}

void UMap3DGameInstanceSubsystem::AddMapZoneVolumeZoneKey(const FGameplayTag& InZoneTag, const FSetElementId& InMapAreaSetIndex)
{
	MapZoneTagToMapAreaSetIndexMap.Add(InZoneTag,InMapAreaSetIndex);
}

AMapArea* UMap3DGameInstanceSubsystem::GetCurrentActiveMapArea()
{
	if(!RegisteredMapAreas.IsEmpty())
	{
		return RegisteredMapAreas[CurrentActiveMapAreaIndex];
	}


	return nullptr;
}

void UMap3DGameInstanceSubsystem::ActivateMapArea(FSetElementId InSetElementIndex)
{
	if(InSetElementIndex.IsValidId() && RegisteredMapAreas[InSetElementIndex])
	{
		CurrentActiveMapAreaIndex = InSetElementIndex;

		if(!RegisteredMapAreas[InSetElementIndex]->IsMapAreaDiscovered())
		{
			AddComponentsToMapView(RegisteredMapAreas[InSetElementIndex]->GetTerrainMeshComponent());
			AddComponentsToMapView(RegisteredMapAreas[InSetElementIndex]->GetAccentMeshComponent());
			
			FGameplayTag ZoneTag;
			RegisteredMapAreas[InSetElementIndex]->GetZoneTag(ZoneTag);

			AddMapProxiesToView(ZoneTag);

			RegisteredMapAreas[InSetElementIndex]->DiscoverMapArea();
		}
	}
}

//--------------------------------------------------------------------------------------------

// Protected Functions --------------------------------------------------------------------------------------------

const FTransform& UMap3DGameInstanceSubsystem::GetCurrentActiveMapAreaInverseTransform()
{
	if (!RegisteredMapAreas.IsEmpty())
	{
		return MapAreaRootInverseTransforms[CurrentActiveMapAreaIndex.AsInteger()];
	}
	return FTransform::Identity;
}

void UMap3DGameInstanceSubsystem::InitialiseWorldDelegates()
{	
	// Binding on OnWorldInitializedActors actors delegate
	FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &UMap3DGameInstanceSubsystem::OnWorldInitializedActors);

	// Binding on OnPostWorldCleanup delegate
	FWorldDelegates::OnPostWorldCleanup.AddUObject(this, &UMap3DGameInstanceSubsystem::OnWorldCleanup);

	#if WITH_EDITOR
	// Binding on OnCurrentLevelChanged delegate
	FWorldDelegates::OnCurrentLevelChanged.AddUObject(this, &UMap3DGameInstanceSubsystem::OnLevelChanged);
	#endif // WITH_EDITOR
}

void UMap3DGameInstanceSubsystem::DeinitialiseWorldDelegates()
{
	// Unbinding on OnWorldInitializedActors actors delegate
	FWorldDelegates::OnWorldInitializedActors.RemoveAll(this);

	// Unbinding on OnPostWorldCleanup delegate
	FWorldDelegates::OnPostWorldCleanup.RemoveAll(this);

	#if WITH_EDITOR
	// Unbinding on OnCurrentLevelChanged delegate
	FWorldDelegates::OnCurrentLevelChanged.RemoveAll(this);
	#endif // WITH_EDITOR
}

void UMap3DGameInstanceSubsystem::OnWorldInitializedActors(const UWorld::FActorsInitializedParams& Params)
{
	if (Params.World)
	{
		UE_LOG(LogTemp, Display, TEXT("Map3D - Actors initialized in world: %s, calculating map terrain transform and registering static map objects"), *Params.World->GetName());
		RegisterStaticMapElements();
	}
}

#if WITH_EDITOR
void UMap3DGameInstanceSubsystem::OnLevelChanged(ULevel* InNewLevel, ULevel* InOldLevel, UWorld* InWorld)
{
	UE_LOG(LogTemp, Display, TEXT("Map3D - Main Level has changed to: %s, reseting registered map elements and other parameters"), *InNewLevel->GetName());
	CleanupRegisteredItemsAndParameters();
}
#endif // WITH_EDITOR

void UMap3DGameInstanceSubsystem::OnWorldCleanup(UWorld* InWorld, bool bSessionEnded, bool bCleanupResources)
{
	if (InWorld)
	{
		UE_LOG(LogTemp, Display, TEXT("Map3D - Main Level post clean up reseting : %s, reseting registered map elements and other parameters"), *InWorld->GetName());
		CleanupRegisteredItemsAndParameters();
	}
}

void UMap3DGameInstanceSubsystem::CleanupRegisteredItemsAndParameters()
{
	bIsMapOpen = false;

	CurrentActiveMapAreaIndex = FSetElementId();

	MapProxyActorsMap.Empty();
	RegisteredObjectiveMapMarkers.Empty();
	RegisteredMapAreas.Empty();
	RegisteredMapZoneVolumes.Empty();
	MapAreaRootInverseTransforms.Empty();
	MapZoneTagToMapAreaSetIndexMap.Empty();
	
	RegisteredPlayerMarker = nullptr;
	RegisteredMapViewer = nullptr;

	ObjectiveMarkerSettings = nullptr;
	MapAreaMaterialSettings = nullptr;
	PlayerMapMarkerSettings = nullptr;
}

//--------------------------------------------------------------------------------------------