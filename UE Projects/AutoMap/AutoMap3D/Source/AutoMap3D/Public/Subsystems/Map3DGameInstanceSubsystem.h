#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Map3DGameInstanceSubsystem.generated.h"

class AMapViewer;
class AMapArea;
class AMapMarkerBase;
class AObjectiveMapMarker;
class UObjectiveMarkerSettings;
class UMapAreaMaterialSettings;
class UPlayerMarkerSettings;
class AMapProxyActor;
class AMapZoneVolume;
enum class ECameraZoomType : uint8;
enum class EObjectiveMapMarkerType : uint8;
enum class EObjectiveMapMarkerDeletionType : uint8;

USTRUCT(BlueprintType)
struct FObjectiveMapMarkerSpawnPair
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AObjectiveMapMarker* MapWorldObjectiveMarker{nullptr};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AObjectiveMapMarker* GameWorldObjectiveMarker{nullptr};
};

USTRUCT(BlueprintType)
struct FMapProxyZoneContainer
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TMap<AMapProxyActor*, AActor*> MapProxyActorsToParentActors{};
};

UCLASS()
class AUTOMAP3D_API UMap3DGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Activation")
	bool CloseMap();

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Activation")
	bool OpenMap();

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Activation")
	bool IsMapOpen() const;

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Navigation")
	void RotateMapView(FVector2f Delta);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Navigation")
	void ZoomMapView(ECameraZoomType ZoomType);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Navigation")
	void PanMapViewXY(FVector2f Delta);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Navigation")
	void PanMapViewUp();

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Navigation")
	void PanMapViewDown();

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Navigation")
	void FocusPlayerMarker();

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Paint")
	void PaintMapArea(const FVector& PaintLocation, float InTerrainPaintRadius, float InBuildingPaintRadius);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Utilities")
	void CalculateMapWorldPosition(const FVector& InWorldPosition, FVector& OutMapPosition);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Utilities")
	void GetPlayerMarkerPosition(FVector& OutPlayerMarkerPosition);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Utilities")
	void GetPlayerMarkerRotation(FRotator& OutPlayerMarkerRotation);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Utilities")
	void GetMapViewerPosition(FVector& OutMapViewerPosition);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Utilities")
	void GetMapViewerForwardDir(FVector& OutMapViewerForward);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Dynamic Map Objects|Map Markers")
	FObjectiveMapMarkerSpawnPair SpawnObjectiveMarker(const FVector& InSpawnLocation, EObjectiveMapMarkerType ObjectiveMarkerType);

	UFUNCTION(Category = "Map 3D System|Dynamic Map Objects")
	void RemoveObjectFromMapView(AActor* InActor);

	UFUNCTION(Category = "Map 3D System|Dynamic Map Objects")
	void RemoveObjectiveMarker(AActor* InActor);

	UFUNCTION(Category = "Map 3D System|Dynamic Map Objects|Map Proxy")
	const AActor* GetProxyParent(AMapProxyActor* InMapProxyActor);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Dynamic Map Objects")
	float GetObjectiveMarkerDeleteDistance();

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|FOV")
	void GetMapViewerFOV(float& OutFOV);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|FOV")
	void GetMapViewerMaxFOV(float& OutFOV);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|FOV")
	void SetMapViewerFOV(float InFOV);
	
	UFUNCTION(BlueprintCallable, Category = "Map 3D System|FOV")
	void SetMapViewerFOVUsingAlpha(float InAlpha);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Player Follow Distance")
	void GetMapViewerPlayerFollowBackwardsOffset(float& OutOffset);
	
	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Player Follow Distance")
	void GetMapViewerPlayerFollowUpwardsOffset(float& OutOffset);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Player Follow Distance")
	float GetMaxMapViewerPlayerFollowDistance();

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Player Follow Distance")
	void SetMapViewerPlayerFollowBackwardsOffset(float InOffset);
	
	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Player Follow Distance")
	void SetMapViewerPlayerFollowUpwardsOffset(float InOffset);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Player Follow Distance")
	void SetMapViewerPlayerFollowBackwardsOffsetUsingAlpha(float InAlpha);
	
	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Player Follow Distance")
	void SetMapViewerPlayerFollowUpOffsetUsingAlpha(float InAlpha);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Zoom")
	void GetMapZoomOffset(float& OutOffset);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Zoom")
	float GetMaxMapZoomOffset();

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Zoom")
	void SetMapZoomOffset(float InOffset);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Zoom")
	void SetMapZoomOffsetUsingAlpha(float InAlpha);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Map Viewer Render Target")
	void ResizeViewerRenderTarget(int32 InWidth, int32 InHeight);

	UFUNCTION( Category = "Map 3D System|Map Area")
	void ActivateMapArea(AMapArea* InInactiveMapArea);

	void ActivateMapArea(const FGameplayTag InMapZoneTagToActivate);

	UFUNCTION( Category = "Map 3D System|Map Area")
	bool IsMapAreaActive(const AMapArea* InMapAreaToCheck);

	bool IsMapAreaActive(const FGameplayTag InZoneTag);

	UFUNCTION( Category = "Map 3D System|Map Area")
	const FTransform& GetCurrentActiveMapAreaInverseTransform();

protected:
	void RegisterMapViewer(AMapViewer* InMapViewer);
	
	void RegisterMapArea(AMapArea* InMapArea);
	
	void RegisterPlayerMapMarker(AMapMarkerBase* InMapMarker);
	
	void RegisterObjectiveMapMarker(AObjectiveMapMarker* InMapWorldObjectiveMarker, AObjectiveMapMarker* InGameWorldObjectiveMarker);
	
	void RegisterMapZoneVolume(AMapZoneVolume* InMapZoneVolume);

	UFUNCTION(Category = "Map 3D System|Utilities")
	void CalculateMapLocalPosition(const FVector& InWorldPosition, FVector& OutLocalPosition);
	
	void CalculateMapEnvironmentTransform(AMapArea* InMapArea, int32 InTransformIndex);

	UFUNCTION(Category = "Map 3D System|Utilities")
	void RegisterStaticMapElements();

	UFUNCTION(Category = "Map 3D System|Utilities")
	void GetMapElementsWithZoneTag(TArray<AActor*>& TerrainActors, TArray<AActor*>& BuildingActors, const FGameplayTag& InMapAreaZoneTag);

	UFUNCTION(Category = "Map 3D System|Dynamic Map Objects")
	void AddObjectToMapView(AActor* InActor);
	
	void AddComponentsToMapView(UPrimitiveComponent* InComponent);

	UFUNCTION(Category = "Map 3D System|Dynamic Map Objects|Map Markers")
	void SetObjectiveMarkerSettings(UObjectiveMarkerSettings* InSettings);

	UFUNCTION(Category = "Map 3D System|Map Area")
	void SetMapAreaMaterialSettings(UMapAreaMaterialSettings* InSettings);

	UFUNCTION(Category = "Map 3D System|Map Markers")
	void SetPlayerMarkerSettings(UPlayerMarkerSettings* InSettings);

	UFUNCTION(BlueprintCallable, Category = "Map 3D System|Setup")
	void SetUpMap(UObjectiveMarkerSettings* InObjectiveMarkerSettings, UMapAreaMaterialSettings* InMapAreaMaterialSettings, UPlayerMarkerSettings* InPlayerMarkerSettings);

	UFUNCTION(Category = "Map 3D System|Map Area Setup")
	void SetupMapAreaMaterials();

	UFUNCTION(Category = "Map 3D System|Map Area Setup")
	void SetupPlayerMarker();

	UFUNCTION(Category = "Map 3D System|Map Area Setup")
	APlayerMapMarker* SpawnPlayerMapMarker();

	UFUNCTION(Category = "Map 3D System|Map Area Setup")
	UMaterialInstanceDynamic* CreateDynamicTerrainUnwrapMaterial();

	UFUNCTION(Category = "Map 3D System|Map Area Setup")
	UMaterialInstanceDynamic* CreateDynamicTerrainMeshMaterial();

	UFUNCTION(Category = "Map 3D System|Map Area Setup")
	UMaterialInstanceDynamic* CreateDynamicAccentMeshMaterial();

	UFUNCTION(Category = "Map 3D System|Map Area Setup")
	UTextureRenderTarget2D* CreatePaintRenderTarget(int32 InRenderTargetWidth, int32 InRenderTargetHeight);

	UFUNCTION(Category = "Map 3D System|Dynamic Map Objects|Map Markers")
	bool SetObjectiveMarkerMesh(AObjectiveMapMarker* InObjectiveMarker , UStaticMesh* InStaticMesh);

	UFUNCTION(Category = "Map 3D System|Dynamic Map Objects|Map Markers")
	UStaticMesh* GetObjectiveMarkerMesh(EObjectiveMapMarkerType InObjectiveMarkerType);

	UFUNCTION(Category = "Map 3D System|Dynamic Map Objects|Map Markers")
	EObjectiveMapMarkerDeletionType GetObjectiveMarkerDeletionSetting(EObjectiveMapMarkerType InObjectiveMarkerType);

	UFUNCTION(Category = "Map 3D System|Dynamic Map Objects|Map Markers")
	void AttachObjectiveMarkerBehaviourComponents(AObjectiveMapMarker* InObjectiveMarker, EObjectiveMapMarkerType InObjectiveMarkerType);
	
	UFUNCTION(Category = "Map 3D System|Dynamic Map Objects|Map Proxies")
	void RegisterMapProxyActor(FGameplayTag InZoneTag, AMapProxyActor* InMapProxy, AActor* InParentActor);

	UFUNCTION(Category = "Map 3D System|Dynamic Map Objects|Map Proxies")
	void SpawnMapProxies(UWorld* InWorld);

	UFUNCTION(Category = "Map 3D System|Dynamic Map Objects|Map Proxies")
	AMapProxyActor* SpawnMapProxy(UWorld* InWorld, const FGameplayTag& InZoneTag, const FVector& InSpawnLocation, const FRotator& InSpawnRotation, UStaticMesh* InMeshToCopy, bool bIsParented);

	UFUNCTION(Category = "Map 3D System|Dynamic Map Objects|Map Proxies")
	void AddMapProxyZoneKey(const FGameplayTag& InZoneTag);

	UFUNCTION(Category = "Map 3D System|Dynamic Map Objects|Map Proxies")
	void AddMapProxiesToView(const FGameplayTag& InZoneTag);

	void AddMapZoneVolumeZoneKey(const FGameplayTag& InZoneTag, const FSetElementId& InMapAreaSetIndex);

	UFUNCTION( Category = "Map 3D System|Map Area")
	AMapArea* GetCurrentActiveMapArea();

	void ActivateMapArea(FSetElementId InSetElementIndex);

	UFUNCTION( Category = "Map 3D System|Initialization")
	void InitialiseWorldDelegates();

	UFUNCTION( Category = "Map 3D System|Initialization")
	void DeinitialiseWorldDelegates();

	void OnWorldInitializedActors(const UWorld::FActorsInitializedParams& Params);
	
	#if WITH_EDITOR
	void OnLevelChanged(ULevel* InNewLevel, ULevel* InOldLevel, UWorld* InWorld);
	#endif // WITH_EDITOR
	
	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	UFUNCTION( Category = "Map 3D System|Deinitialization")
	void CleanupRegisteredItemsAndParameters();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map 3D System")
	TMap<FGameplayTag, FMapProxyZoneContainer> MapProxyActorsMap{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map 3D System", meta = (ToolTip = "This is a TMap of the Map World Objective Markers (key) to the Game World Objective Markers (value)"))
	TMap<AObjectiveMapMarker*, AObjectiveMapMarker*> RegisteredObjectiveMapMarkers{};

	UPROPERTY(VisibleAnywhere, Category = "Map 3D System")
	TSet<AMapArea*> RegisteredMapAreas{};

	UPROPERTY(VisibleAnywhere, Category = "Map 3D System")
	TArray<AMapZoneVolume*> RegisteredMapZoneVolumes{};

	UPROPERTY(VisibleAnywhere, Category = "Map 3D System")
	TArray<FTransform> MapAreaRootInverseTransforms{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map 3D System")
	UObjectiveMarkerSettings* ObjectiveMarkerSettings{nullptr};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map 3D System")
	UMapAreaMaterialSettings* MapAreaMaterialSettings{nullptr};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map 3D System")
	UPlayerMarkerSettings* PlayerMapMarkerSettings{nullptr};

	TMap<FGameplayTag, FSetElementId> MapZoneTagToMapAreaSetIndexMap{};

	UPROPERTY(VisibleAnywhere, Category = "Map 3D System")
	AMapViewer* RegisteredMapViewer{nullptr};

	UPROPERTY(VisibleAnywhere, Category = "Map 3D System")
	AMapMarkerBase* RegisteredPlayerMarker{nullptr};

	UPROPERTY(VisibleAnywhere, Category = "Map 3D System|Settings")
	bool bIsMapOpen{false};

	FSetElementId CurrentActiveMapAreaIndex;
};