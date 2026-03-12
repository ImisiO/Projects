#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MapAreaMaterialSettings.generated.h"

UCLASS()
class AUTOMAP3D_API UMapAreaMaterialSettings : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UMaterialInterface* GetUnwrapMaterial();
	
	UMaterialInterface* GetTerrainMaterial();
	
	UMaterialInterface* GetAccentMaterial();
	
	UMaterialInterface* GetMapProxyMaterial();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials|Unwrap")
	UMaterialInterface* UnwrapMaterial{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials|Meshes")
	UMaterialInterface* TerrainMaterial{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials|Meshes")
	UMaterialInterface* AccentMaterial{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials|Activatables")
	UMaterialInterface* MapProxyMaterial{nullptr};
};
