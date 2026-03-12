#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerMarkerSettings.generated.h"

UCLASS()
class AUTOMAP3D_API UPlayerMarkerSettings : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UStaticMesh* GetPlayerMapMarkerMesh();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Marker Mesh Settings")
	UStaticMesh* PlayerMapMarkerMesh{nullptr};
};
