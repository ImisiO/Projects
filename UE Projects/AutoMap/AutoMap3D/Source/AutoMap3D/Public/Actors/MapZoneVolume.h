#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayTagContainer.h"
#include "MapZoneVolume.generated.h"

UCLASS()
class AUTOMAP3D_API AMapZoneVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	AMapZoneVolume();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void ActivateMap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	bool IsAssociatedMapAreaActive();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Zone|Volume")
	UBoxComponent* ZoneVolume{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadOnly , Category = "Map Zone|Zone Tag")
	FGameplayTag ZoneTag;
};
