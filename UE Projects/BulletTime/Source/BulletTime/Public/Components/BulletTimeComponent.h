#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BulletTimeComponent.generated.h"

UCLASS(ClassGroup = "BulletTime", meta = (BlueprintSpawnableComponent), HideCategories = (Activation, Cooking, Sockets, Collision))
class BULLETTIME_API UBulletTimeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBulletTimeComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* pThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable, Category = "BulletTime")
    void ApplyPinnedDilation();

	UFUNCTION(BlueprintCallable, Category = "BulletTime")
    void ClearPinnedDilation();

	UFUNCTION(BlueprintCallable, Category = "BulletTime")
    void SetPinnedDilation(float Factor);

	UFUNCTION(BlueprintPure, Category = "BulletTime")
    bool IsPinned() const { return PinnedDilationFactor >= 0.0f; }

	UFUNCTION(BlueprintPure, Category = "BulletTime")
    bool IsImmuneToBTVolumes() const { return bImmuneToVolumes; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "BulletTime")
    bool bImmuneToVolumes = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "BulletTime", meta = (ClampMin = "-1.0", UIMin = "-1.0"))
    float PinnedDilationFactor = -1.0f;
};
