#pragma once

#include "CoreMinimal.h"
#include "MapMarkerBase.h"
#include "ObjectiveMapMarker.generated.h"

UENUM(BlueprintType)
enum class EObjectiveMapMarkerType : uint8
{
	Waypoint_Marker UMETA(DisplayName = "Way Point Marker"),
};

UENUM(BlueprintType)
enum class EObjectiveMapMarkerDeletionType : uint8
{
	Del_Null UMETA(DisplayName = "Marker will be not deleted and will persist in the level until its deleted manually or by some other process"),
	Del_Distance UMETA(DisplayName = "Marker will be deleted based on the distance to the player"),
};

UENUM(BlueprintType)
enum class EObjectiveMapMarkerBehaviour : uint8
{
	Be_None UMETA(DisplayName = "Marker will stay stationary when in view"),
	Be_Rotate UMETA(DisplayName = "Marker will rotate when in view"),
	Be_Bob UMETA(DisplayName = "Marker will bob up and down when in view"),
};

UCLASS()
class AUTOMAP3D_API AObjectiveMapMarker : public AMapMarkerBase
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;

	virtual bool SetMesh(UStaticMesh* InStaticMesh) override;

	void SetMarkerDeletionType(EObjectiveMapMarkerDeletionType InDeletionType);
	
	void GetMarkerDeletionType(EObjectiveMapMarkerDeletionType& OutDeletionType);

	void SetIsObjectiveMarkerInMapWorld(bool bIsInMapWorld);
	
	bool IsObjectMarkerInMapWorld();

protected:
	virtual void BeginPlay() override;

	void HandleDeletionBehaviour();

	UPROPERTY(VisibleAnywhere, Category = "Map Marker|Deletion")
	EObjectiveMapMarkerDeletionType MarkerDeletionType{ EObjectiveMapMarkerDeletionType::Del_Null};

	UPROPERTY(VisibleAnywhere, Category = "Map Marker|Deletion")
	bool bIsMapWorldObjectiveMarker{true};
};
