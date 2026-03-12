#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "MapProxyActor.generated.h"

UCLASS()
class AUTOMAP3D_API AMapProxyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AMapProxyActor();

	virtual void Tick(float DeltaTime) override;

	bool SetMapProxyMesh(UStaticMesh* InMesh);

	bool SetMapProxyMaterial(UMaterialInterface* InMaterial);

	void SetShouldUpdateTransform(bool bInShouldUpdateTransform);

	void GetZoneTag(FGameplayTag& OutGameplayTag);

	void SetZoneTag(const FGameplayTag& InGameplayTag);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Map Proxy")
	UStaticMeshComponent* MapProxyMesh{nullptr};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly , Category = "Map Proxy|Tags")
	FGameplayTag ZoneTag;

	UPROPERTY(VisibleAnywhere, Category = "Map Proxy")
	bool bShouldUpdateTransform{false};
};
