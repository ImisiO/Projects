#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BulletTimeSubsystem.generated.h"

class ABulletTimeVolume;

UCLASS()
class BULLETTIME_API UBulletTimeSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    virtual void Deinitialize() override;

    virtual void Tick(float DeltaTime) override;

    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UBulletTimeSubsystem, STATGROUP_Tickables); }
    
    virtual bool IsTickable() const override { return bIsBlending; }

    UFUNCTION(BlueprintCallable, Category = "BulletTime|Global", meta = (ClampMin = "0.01", ClampMax = "1.0"))
    void ActivateGlobalBulletTime(float DilationFactor = 0.2f, float BlendTime = 0.25f, bool bAffectsPlayer = false);

    UFUNCTION(BlueprintCallable, Category = "BulletTime|Global")
    void DeactivateGlobalBulletTime(float BlendTime = 0.25f);

    UFUNCTION(BlueprintPure, Category = "BulletTime|Global")
    float GetCurrentGlobalDilation() const { return CurrentGlobalDilation; }

    UFUNCTION(BlueprintPure, Category = "BulletTime|Global")
    bool IsGlobalBulletTimeActive() const;

    void RegisterActorInBTVolume(AActor* pActor, ABulletTimeVolume* pBTVolume, bool bVolumeAffectsPlayer);
    
    void UnregisterActorFromBTVolume(AActor* pActor, ABulletTimeVolume* pBTVolume);

private:
    void ApplyDilations();

    void CompensatePlayerPawnTimeDia(float GlobalDilation);

    void ResetPlayerTimeDialation();

    APawn* GetLocalPlayerPawn() const;

    bool ShouldActorBeImmuneToBTVolumes(const AActor* pActor, bool bVolumeAffectsPlayer) const;
    
    TMap<TWeakObjectPtr<AActor>, TArray<TWeakObjectPtr<ABulletTimeVolume>>> ActorVolumeStack;

    TWeakObjectPtr<APawn> CachedPlayerPawn;

    float TargetGlobalDilation = 1.0f;

    float CurrentGlobalDilation = 1.0f;

    float BlendInterpSpeed = 10.0f;

    bool bIsBlending = false;

    bool bCurrentActivationAffectsPlayer = false;
};
