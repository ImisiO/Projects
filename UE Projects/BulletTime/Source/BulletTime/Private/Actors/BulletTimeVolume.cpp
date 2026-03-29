#include "Actors/BulletTimeVolume.h"
#include "Subsystems/BulletTimeSubsystem.h"
#include "Components/BrushComponent.h"
#include "DrawDebugHelpers.h"

ABulletTimeVolume::ABulletTimeVolume()
{
    if (UBrushComponent* BrushComp = GetBrushComponent())
    {
        BrushComp->SetCollisionProfileName(TEXT("OverlapAll"));
        BrushComp->SetGenerateOverlapEvents(true);
        BrushComp->bAlwaysCreatePhysicsState = true;
    }

    PrimaryActorTick.bCanEverTick = true;
}

void ABulletTimeVolume::Tick(float DeltaTime)
{
#if ENABLE_DRAW_DEBUG
    if (!bDrawDebug)
    {
        return;
    }

    const FBox Bounds = GetComponentsBoundingBox(true);
    const FVector Center = Bounds.GetCenter();
    const FVector Extent = Bounds.GetExtent();

    DrawDebugBox(
        GetWorld(),
        Center,
        Extent,
        FQuat::Identity,
        DebugColor.ToFColor(true),
        false,   
        -1.0f,   
        0,
        2.0f    
    );

    DrawDebugString(
        GetWorld(),
        Center,
        FString::Printf(TEXT("Dilation: %.2f\nOccupants: %d"), TimeDilationFactor, Occupants.Num()),
        nullptr,
        FColor::White,
        0.0f,   
        true     
    );
#endif
}

bool ABulletTimeVolume::ContainsActor(const AActor* pOtherActor) const
{
    if (!pOtherActor)
    { 
        return false;
    }

    return Occupants.Contains(const_cast<AActor*>(pOtherActor));
}

TArray<AActor*> ABulletTimeVolume::GetOccupants() const
{
    TArray<AActor*> Result;
    Result.Reserve(Occupants.Num());

    for (const TWeakObjectPtr<AActor>& pOccupant : Occupants)
    {
        if (AActor* Actor = pOccupant.Get())
        {
            Result.Add(Actor);
        }
    }

    return Result;
}

void ABulletTimeVolume::NotifyActorBeginOverlap(AActor* pOtherActor)
{
    Super::NotifyActorBeginOverlap(pOtherActor);

    if (!IsValid(pOtherActor))
    { 
        return;
    }

    if (!bAffectsPlayer)
    {
        if (const APlayerController* pPlayerController = pOtherActor->GetWorld()->GetFirstPlayerController())
        {
            if (pPlayerController->GetPawn() == pOtherActor) 
            {
                return;
            }
        }
    }

    Occupants.Add(pOtherActor);

    if (UBulletTimeSubsystem* pBTSubsystem = GetWorld()->GetSubsystem<UBulletTimeSubsystem>())
    {
        pBTSubsystem->RegisterActorInBTVolume(pOtherActor, this, bAffectsPlayer);
    }
}

void ABulletTimeVolume::NotifyActorEndOverlap(AActor* pOtherActor)
{
    Super::NotifyActorEndOverlap(pOtherActor);

    if (!IsValid(pOtherActor)) 
    {
        return;
    }

    Occupants.Remove(pOtherActor);

    if (UBulletTimeSubsystem* pBTSubsystem = GetWorld()->GetSubsystem<UBulletTimeSubsystem>())
    {
        pBTSubsystem->UnregisterActorFromBTVolume(pOtherActor, this);
    }
}

#if WITH_EDITOR

void ABulletTimeVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ABulletTimeVolume, TimeDilationFactor))
    {
        if (UBulletTimeSubsystem* pBTSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UBulletTimeSubsystem>() : nullptr)
        {
            for (const TWeakObjectPtr<AActor>& pOccupant : Occupants)
            {
                if (AActor* pActor = pOccupant.Get())
                {
                    pBTSubsystem->UnregisterActorFromBTVolume(pActor, this);
                    pBTSubsystem->RegisterActorInBTVolume(pActor, this, bAffectsPlayer);
                }
            }
        }
    }
}

#endif