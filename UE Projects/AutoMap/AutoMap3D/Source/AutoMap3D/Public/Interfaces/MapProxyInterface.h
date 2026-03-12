#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "MapProxyInterface.generated.h"

class AMapProxyActor;

UINTERFACE(MinimalAPI, Blueprintable)
class UMapProxyInterface : public UInterface
{
	GENERATED_BODY()
};

class AUTOMAP3D_API IMapProxyInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Map Proxy Interface|Position")
	FVector GetGameWorldPosition();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Map Proxy Interface|Mesh")
	UStaticMesh* GetMapProxyMesh();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Map Proxy Interface|Transform", meta = (ToolTip = "This will be a check to see if the object needs to be parented in terms of position (moving objects) and rotation"))
	bool ShouldUpdateTransform();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Map Proxy Interface|Gameplay Tag", meta = (ToolTip = "This is used to bind the spawned map proxy to a map zone"))
	FGameplayTag GetMapZoneGamePlayTag();
};
