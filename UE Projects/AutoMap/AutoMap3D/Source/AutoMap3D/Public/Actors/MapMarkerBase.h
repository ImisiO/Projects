#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapMarkerBase.generated.h"

UCLASS()
class AUTOMAP3D_API AMapMarkerBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AMapMarkerBase();

	virtual void Tick(float DeltaTime) override;

	virtual bool SetMesh(UStaticMesh* InStaticMesh);

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(CallInEditor, Category = "Map Marker")
	void ToggleMeshSceneCaptureVisibility();

	UFUNCTION(Category = "Map Marker|Utilities")
	void GetMapPosition(const FVector& InVector, FVector& OutVector);

	UPROPERTY(EditAnywhere, Category = "Map Marker")
	UStaticMeshComponent* MarkerMesh{nullptr};

	UPROPERTY(BlueprintReadOnly, Category = "Map Marker|Settings")
	bool bIsMarkerMeshVisibleInSceneCapture{false};
};
