#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "GameplayTagContainer.h"
#include "Components/SceneCaptureComponent2D.h"
#include "MapArea.generated.h"

UCLASS()
class AUTOMAP3D_API AMapArea : public AActor
{
	GENERATED_BODY()
	
public:	
	AMapArea();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Category = "Map Area|Activation")
	void OpenMap();

	UFUNCTION(Category = "Map Area|Activation")
	void CloseMap();

	UFUNCTION(Category = "Map Area|Paint")
	void PaintArea(const FVector& PaintLocation, float InTerrainPaintRadius);

	UFUNCTION(Category = "Map Area|Utilities|Setup")
	void SetupPreBeginPlayParameters();

	UFUNCTION(Category = "Map Area|Utilities|Setup")
	void SetupMaterialsParameters();

	UFUNCTION(Category = "Map Area|Paint|Materials")
	void SetDynamicAccentMeshMaterial(UMaterialInstanceDynamic* InDynamicMaterial);

	UFUNCTION(Category = "Map Area|Paint|Materials")
	void SetDynamicTerrainUnwrapMaterial(UMaterialInstanceDynamic* InDynamicMaterial);

	UFUNCTION(Category = "Map Area|Paint|Materials")
	void SetDynamicTerrainMeshMaterial(UMaterialInstanceDynamic* InDynamicMaterial);

	UFUNCTION(Category = "Map Area|Paint|RenderTargets")
	void SetTerrainPaintTextureTarget(UTextureRenderTarget2D* InNewTextureRenderTarget);

	UFUNCTION()
	UStaticMeshComponent* GetTerrainMeshComponent();

	UFUNCTION()
	UStaticMeshComponent* GetAccentMeshComponent();

	void GetZoneTag(FGameplayTag& OutGameplayTag);

	void DiscoverMapArea();
	bool IsMapAreaDiscovered();

	UFUNCTION(Category = "Map Area|Utilities")
	void CalculateMapEnvironmentTerrainPosition(FVector& OutTerrainPosition);

	bool IsUsingHighResolutionRT();

	FVector GetGameWorldOrigin() const { return GameWorldOrigin; }

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(CallInEditor, Category = "Map Area|Utilities|Editor")
	void ToggleMeshSceneCaptureVisibility();

	UFUNCTION(CallInEditor, Category = "Map Area|Utilities|Editor")
	void AlignAccentMesh();

	UFUNCTION(CallInEditor, Category = "Map Area|Utilities|Editor")
	void AlignMapAreas();

	UFUNCTION(CallInEditor, Category = "Map Area|Utilities|Editor")
	void BakeGameWorldOrigin();

	UFUNCTION(Category = "Map Area|Utilities")
	void SetMeshComponentSceneCaptureVisibility(bool InNewState);

	UFUNCTION(Category = "Map Area|Utilities|Editor")
	void SetAccentRelativePosition(const FVector& InVector);

	UFUNCTION(Category = "Map Area|Utilities|Setup")
	void CalculateViewParameters(float& OutOrthoWidth, FVector& OutRelativeLocatio, FVector& OutUnrwapLocation);

	UFUNCTION(Category = "Map Area|Utilities|Setup")
	void SetupCaptureParameters();

	UFUNCTION(Category = "Map Area|Utilities|Setup")
	void SetupTerrainUnwrapMesh();

	UFUNCTION(Category = "Map Area|Paint")
	void HandlePaintBehaviour(float DeltaTime);

	UFUNCTION(Category = "Map Area|Occlusion")
	void HandleViewOcclusionBehaviour();

	UFUNCTION(Category = "Map Area|Paint|Utilities")
	bool AreUnwrapMaterialsValid();

	UFUNCTION(Category = "Map Area|Paint|Utilities")
	bool AreMeshMaterialsValid();

	UFUNCTION(Category = "Map Area|Paint|Utilities")
	bool GetPlayerMarkerPosition(FVector& OutVector);

	UFUNCTION(Category = "Map Area|Occlusion|Utilities")
	bool GetMapViewerPosition(FVector& OutVector);

	UFUNCTION(Category = "Map Area|Occlusion|Utilities")
	bool IsActiveMapArea();

	UPROPERTY(EditAnywhere, Category = "Map Area|Meshes|Terrain")
	UStaticMeshComponent* TerrainMeshComp{nullptr};

	UPROPERTY(EditAnywhere, Category = "Map Area|Meshes|Accent")
	UStaticMeshComponent* AccentMeshComp{nullptr};

	UPROPERTY()
	UStaticMeshComponent* TerrainUnwrapMesh{nullptr};

	UPROPERTY(BlueprintReadOnly, Category = "Map Area|Paint|Capture")
	USceneCaptureComponent2D* TerrainPaintCapture{nullptr};

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicTerrainUnwrapMaterial{nullptr};

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicTerrainMaterial{nullptr};

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicAccentMaterial{nullptr};

	UPROPERTY()
	UMaterialInterface* SwapMaterial{nullptr};

	UPROPERTY()
	USceneComponent* TerrainSceneComponent{nullptr};

	UPROPERTY()
	USceneComponent* AccentSceneComponent{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadOnly , Category = "Map Area|Tags")
	FGameplayTag ZoneTag;

	FVector UnwrapLocation{};

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Map Area|Paint|Settings", meta = (UIMin = "200", UIMax = "1000", ClampMin = "200", ClampMax = "1000", ToolTip = "This is the radius when painting on the terrain mesh. Be careful not to set it too high as when painting it paints in a sphere rather just a 2d circle."))
	float TerrainPaintRadius{500.0f};

	UPROPERTY(BlueprintReadWrite, Category = "Map Area|Paint|Settings")
	float PaintHardness{0.25f};

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Map Area|Paint|Settings")
	float PaintRate{0.25f};

	UPROPERTY()
	float PaintTimer{0.0f};

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Map Area|Occlusion|Settings")
	float OcclusionMaxDistance{1500.0f};

	UPROPERTY(BlueprintReadOnly, Category = "Map Area|Settings|Editor")
	bool bIsMeshOnlyVisibleInSceneCapture{false};

	UPROPERTY(BlueprintReadOnly, Category = "Map Area|Settings|State")
	bool bIsMapOpen{false};

	UPROPERTY(EditInstanceOnly, Category = "Map Area|Paint", meta = (ToolTip = "Switching this to true will mean that this map area will use a 512 x 512 render target rather than a 256 x 256 render target"))
	bool bIsUsingHighResolutionRT{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map Area")
	bool bHasBeenDiscovered{false};

	UPROPERTY(EditInstanceOnly, Category = "Map Area|Calibration")
	FVector GameWorldOrigin{ FVector::ZeroVector };

private:
	void GetTaggedMapElements(TArray<AActor*>& TerrainActors, TArray<AActor*>& BuildingActors);
};
