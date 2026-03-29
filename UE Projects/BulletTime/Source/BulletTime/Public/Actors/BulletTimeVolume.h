#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "BulletTimeVolume.generated.h"

UCLASS(Blueprintable, BlueprintType, HideCategories = (Collision))
class BULLETTIME_API ABulletTimeVolume : public AVolume
{
	GENERATED_BODY()
public:
 
    ABulletTimeVolume();

    virtual void Tick(float DeltaTime) override;
    
    UFUNCTION(BlueprintPure, Category = "BulletTime")
    const float& GetTimeDilationFactor() const { return TimeDilationFactor; }

    UFUNCTION(BlueprintPure, Category = "BulletTime")
    bool ContainsActor(const AActor* pOtherActor) const;

	UFUNCTION(BlueprintPure, Category = "BulletTime")
	TArray<AActor*> GetOccupants() const;
 
protected:
    virtual void NotifyActorBeginOverlap(AActor* pOtherActor) override;
    
    virtual void NotifyActorEndOverlap(AActor* pOtherActor) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
 
    TSet<TWeakObjectPtr<AActor>> Occupants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BulletTime|Debug", meta = (EditCondition = "bDrawDebug"))
    FLinearColor DebugColor = FLinearColor(0.1f, 0.4f, 1.0f, 0.35f);

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "BulletTime", meta = (ClampMin = "0.01", UIMin = "0.01"))
    float TimeDilationFactor = 0.25f;
 
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "BulletTime")
    bool bAffectsPlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BulletTime|Debug")
    bool bDrawDebug = false;
};
