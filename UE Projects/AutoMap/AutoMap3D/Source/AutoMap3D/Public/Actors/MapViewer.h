#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/Actor.h"
#include "MapViewer.generated.h"

UENUM(BlueprintType)
enum class ECameraZoomType : uint8
{
	Zoom_Out UMETA(DisplayName = "Zoom Out"),
	Zoom_In UMETA(DisplayName = "Zoom In"),
};

UENUM(BlueprintType)
enum class EMapViewerState : uint8
{
	Default UMETA(DisplayName = "Default State"),
	Focusing UMETA(DisplayName = "Focus State"),
};

UCLASS()
class AUTOMAP3D_API AMapViewer : public AActor
{
	GENERATED_BODY()
	
public:	
	AMapViewer();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Category = "Map Viewer|Navigation")
	void ApplyZoom(ECameraZoomType ZoomType);

	UFUNCTION(Category = "Map Viewer|Navigation")
	void ApplyRotation(FVector2f Delta);

	UFUNCTION(Category = "Map Viewer|Navigation")
	void ApplyPanXY(FVector2f Delta);

	UFUNCTION(Category = "Map Viewer|Navigation")
	void ApplyPanUp();
		
	UFUNCTION(Category = "Map Viewer|Navigation")
	void ApplyPanDown();

	UFUNCTION(Category = "Map Viewer|Navigation")
	void FocusPlayer();

	UFUNCTION(Category = "Map Viewer|Activation")
	void OpenMap();

	UFUNCTION(Category = "Map Viewer|Activation")
	void CloseMap();
	
	UFUNCTION(Category = "Map Viewer|FOV")
	void SetFOV(float InFOV);

	UFUNCTION(Category = "Map Viewer|FOV")
	float GetFOV();

	UFUNCTION(Category = "Map Viewer|FOV")
	float GetMaxFOV();

	UFUNCTION(Category = "Map Viewer|Player Follow Distance")
	void SetPlayerFollowUpwardsOffset(float InOffset);

	UFUNCTION(Category = "Map Viewer|Player Follow Distance")
	float GetPlayerFollowUpwardsOffset();

	UFUNCTION(Category = "Map Viewer|Player Follow Distance")
	void SetPlayerFollowBackwardsOffset(float InOffset);

	UFUNCTION(Category = "Map Viewer|Player Follow Distance")
	float GetPlayerFollowBackwardsOffset();

	UFUNCTION(Category = "Map Viewer|Zoom")
	void SetZoomDistanceOffset(float InOffset);

	UFUNCTION(Category = "Map Viewer|Zoom")
	float GetZoomDistanceOffset();

	UFUNCTION(Category = "Map Viewer|Zoom")
	float GetMaxZoomDistance();

	UFUNCTION(Category = "Map Viewer|Dynamic Objects")
	void AddObjectToMapView(AActor* InActor);
		
	UFUNCTION(Category = "Map Viewer|Dynamic Objects")
	void AddComponentToMapView(UPrimitiveComponent* InComponent);

	UFUNCTION(Category = "Map Viewer|Dynamic Objects")
	void RemoveObjectFromMapView(AActor* InActor);

	UFUNCTION(Category = "Map Viewer|Render Target")
	void ResizeViewerRenderTarget(int32 InWidth, int32 InHeight);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(Category = "Map Viewer|Utilities")
	void FollowPlayer();

	UFUNCTION(Category = "Map Viewer|Utilities")
	void FocusPosition(const FVector& PositonToFocus, const FVector& CameraTargetPosition);

	UFUNCTION(Category = "Map Viewer|Utilities")
	void InterruptFocusingState();

	UFUNCTION(Category = "Map Viewer|Utilities")
	void HandleFocusingPositionBehaviour();

	UFUNCTION(Category = "Map Viewer|Utilities")
	bool HasFinishedFocusing();

	UFUNCTION(Category = "Map Viewer|Utilities")
	void GetPlayerMarkerPosition(FVector& OutPlayerPosition);

	UFUNCTION(Category = "Map Viewer|Utilities")
	float GetZoomRatio(float CurrentZoomDistance);

	UFUNCTION(Category = "Map Viewer|Rotation")
	bool IsPositionWithRotationRange(const FVector& NewLocation);

	UFUNCTION(Category = "Map Viewer|Zoom")
	void CalculateRotationAngles();
	
	UFUNCTION(Category = "Map Viewer|Zoom")
	void CalculateNewDefaultZoomDistance();

	void CaptureScene();

	UPROPERTY(EditAnywhere, Category = "Map Viewer")
	USceneCaptureComponent2D* MapCameraView{nullptr};
	
	UPROPERTY()
	USceneComponent* MapViewSceneComponent{nullptr};

	UPROPERTY(EditAnywhere, Category = "Map Viewer|Settings|Zoom", meta = (ToolTip = "Curve used to drive zoom lerp"))
	UCurveFloat* ZoomCurve{nullptr};

	UPROPERTY(VisibleAnywhere, Category = "Map Viewer|Settings")
	FVector CameraFocusPosition;

	UPROPERTY(VisibleAnywhere, Category = "Map Viewer|Settings|Focus")
	FVector NewCameraPosition;

	UPROPERTY(EditAnywhere, Category = "Map Viewer|Settings|Panning")
	float PanningSpeedXY{25.0f};

	UPROPERTY(EditAnywhere, Category = "Map Viewer|Settings|Panning")
	float PanningSpeedZ{20.0f};

	UPROPERTY(EditAnywhere, Category = "Map Viewer|Settings|Zoom")
	float ZoomSpeed{10.0f};

	UPROPERTY(EditAnywhere, Category = "Map Viewer|Settings|Zoom", meta = (ToolTip = "Only used when there is a float curve given"))
	float ZoomCurveMagnitude{16.0f};

	UPROPERTY(EditAnywhere, Category = "Map Viewer|Settings|Zoom", meta = (ToolTip = "This will define the maximum zoom distance based on the DefaultZoomDistance"))
	float ZoomDistanceOffset{1000.0f};

	UPROPERTY(VisibleAnywhere, Category = "Map Viewer|Settings|Zoom")
	float DefaultZoomDistance{0.0f};

	UPROPERTY(EditAnywhere, Category = "Map Viewer|Settings|Zoom")
	float MinZoomDistance{1000.0f};

	UPROPERTY(EditAnywhere, Category = "Map Viewer|Settings|Rotation")
	float RotationSpeed{5.0f};

	float ElevationAngle{0.0f};

	float AzimuthAngle{0.0f};

	UPROPERTY(VisibleAnywhere, Category = "Map Viewer|Settings|Focus")
	float FocusPointLerpSpeed{40.0f};

	UPROPERTY(EditAnywhere, Category = "Map Viewer|Settings")
	float DefaultFieldOfView{100.0f};

	UPROPERTY(EditAnywhere, Category = "Map Viewer|Settings")
	float MaxFieldOfView{179.0f};

	UPROPERTY(EditAnywhere, Category = "Map Viewer|Settings|Player Follow", meta = (ToolTip = "How far up the Map Viewer will be when following the player"))
	float PlayerFollowUpwardsOffset{4000.0f};

	UPROPERTY(EditAnywhere, Category = "Map Viewer|Settings|Player Follow", meta = (ToolTip = "How far back the Map Viewer will be when following the player"))
	float PlayerFollowBackwardsOffset{1000.0f};

	UPROPERTY(EditAnywhere, Category = "Map Viewer|Settings|Player Follow")
	float PlayerFollowMovementLerpSpeed{8.0f};

	UPROPERTY(EditAnywhere, Category = "Map Viewer|Settings|Player Follow")
	float PlayerFollowRotateLerpSpeed{5.0f};

	UPROPERTY(VisibleAnywhere, Category = "Map Viewer|Settings")
	bool bIsMapOpen{false};

	UPROPERTY(VisibleAnywhere, Category = "Map Viewer|Settings|State")
	EMapViewerState MapViewerState{EMapViewerState::Default};
};
