#pragma once

#include "CoreMinimal.h"
#include "MapMarkerBase.h"
#include "PlayerMapMarker.generated.h"

UCLASS()
class AUTOMAP3D_API APlayerMapMarker : public AMapMarkerBase
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(Category = "Map Marker")
	void FollowPlayer();
};
