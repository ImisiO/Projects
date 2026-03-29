#include "Subsystems/BulletTimeSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Actors/BulletTimeVolume.h"
#include "Components/BulletTimeComponent.h"

void UBulletTimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CurrentGlobalDilation = 1.0f;
    TargetGlobalDilation = 1.0f;
    bIsBlending = false;

    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
}

void UBulletTimeSubsystem::Deinitialize()
{
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
   
    ResetPlayerTimeDialation();
    ActorVolumeStack.Empty();

    Super::Deinitialize();
}

void UBulletTimeSubsystem::Tick(float DeltaTime)
{
    const float RealDelta = GetWorld()->GetDeltaSeconds() / FMath::Max(CurrentGlobalDilation, 0.001f);

    CurrentGlobalDilation = FMath::FInterpTo(
        CurrentGlobalDilation,
        TargetGlobalDilation,
        RealDelta,
        BlendInterpSpeed);

    if (FMath::IsNearlyEqual(CurrentGlobalDilation, TargetGlobalDilation, 0.002f))
    {
        CurrentGlobalDilation = TargetGlobalDilation;
        bIsBlending = false;
    }

    ApplyDilations();
}

void UBulletTimeSubsystem::ActivateGlobalBulletTime(float DilationFactor, float BlendTime, bool bAffectsPlayer)
{
    TargetGlobalDilation = DilationFactor;
    bCurrentActivationAffectsPlayer = bAffectsPlayer;

    if (BlendTime <= 0.0f)
    {
        CurrentGlobalDilation = TargetGlobalDilation;
        bIsBlending = false;
        ApplyDilations();
    }
    else
    {
        BlendInterpSpeed = 4.0f / FMath::Max(BlendTime, 0.001f);
        bIsBlending = true;
    }
}

void UBulletTimeSubsystem::DeactivateGlobalBulletTime(float BlendTime)
{
    ActivateGlobalBulletTime(1.0f, BlendTime);
}

bool UBulletTimeSubsystem::IsGlobalBulletTimeActive() const
{
    return !FMath::IsNearlyEqual(CurrentGlobalDilation, 1.0f, 0.001f)
        || !FMath::IsNearlyEqual(TargetGlobalDilation, 1.0f, 0.001f);
}

void UBulletTimeSubsystem::RegisterActorInBTVolume(AActor* pActor, ABulletTimeVolume* pBTVolume, bool bVolumeAffectsPlayer)
{
    if (!IsValid(pActor) || !IsValid(pBTVolume)) 
    {
        return;
    }

    if (ShouldActorBeImmuneToBTVolumes(pActor, bVolumeAffectsPlayer))
    {
        return;
    }

    TArray<TWeakObjectPtr<ABulletTimeVolume>>& Stack = ActorVolumeStack.FindOrAdd(pActor);
    Stack.AddUnique(pBTVolume);

    pActor->CustomTimeDilation = pBTVolume->GetTimeDilationFactor();
}

void UBulletTimeSubsystem::UnregisterActorFromBTVolume(AActor* pActor, ABulletTimeVolume* pBTVolume)
{
    if (!IsValid(pActor)) 
    {
        return;
    }

    TArray<TWeakObjectPtr<ABulletTimeVolume>>* Stack = ActorVolumeStack.Find(pActor);
    if (!Stack)
    {
        return;
    }

    Stack->RemoveSingle(pBTVolume);

    if (Stack->Num() == 0)
    {
        ActorVolumeStack.Remove(pActor);
        pActor->CustomTimeDilation = 1.0f;
    }
    else
    {
        if (ABulletTimeVolume* PreviousVolume = Stack->Last().Get())
        {
            pActor->CustomTimeDilation = PreviousVolume->GetTimeDilationFactor();
        }
    }
}

void UBulletTimeSubsystem::ApplyDilations()
{
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), CurrentGlobalDilation);

    if (!bCurrentActivationAffectsPlayer)
    {
        CompensatePlayerPawnTimeDia(CurrentGlobalDilation);
    }
}

void UBulletTimeSubsystem::CompensatePlayerPawnTimeDia(float GlobalDilation)
{
    APawn* Player = GetLocalPlayerPawn();
    if (!Player)
    { 
        return;
    }

    Player->CustomTimeDilation = 1.0f / FMath::Max(GlobalDilation, 0.001f);

    if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
    {
        PC->CustomTimeDilation = Player->CustomTimeDilation;
    }
}

void UBulletTimeSubsystem::ResetPlayerTimeDialation()
{
    if (APawn* Player = GetLocalPlayerPawn())
    {
        Player->CustomTimeDilation = 1.0f;

        if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
        {
            PC->CustomTimeDilation = 1.0f;
        }
    }
}

APawn* UBulletTimeSubsystem::GetLocalPlayerPawn() const
{
    if (CachedPlayerPawn.IsValid()) 
    {
        return CachedPlayerPawn.Get();
    }

    const UWorld* World = GetWorld();
    if (!World) 
    {
        return nullptr;
    }

    if (APlayerController* PC = World->GetFirstPlayerController())
    {
        const_cast<UBulletTimeSubsystem*>(this)->CachedPlayerPawn = PC->GetPawn();
        return CachedPlayerPawn.Get();
    }

    return nullptr;
}

bool UBulletTimeSubsystem::ShouldActorBeImmuneToBTVolumes(const AActor* pActor, bool bVolumeAffectsPlayer) const
{
    if (!pActor) 
    {
        return false;
    }

    if (!bVolumeAffectsPlayer && pActor == GetLocalPlayerPawn())
    {
        return true;
    }

    if (const UBulletTimeComponent* BTC = pActor->FindComponentByClass<UBulletTimeComponent>())
    {
        return BTC->IsImmuneToBTVolumes();
    }

    return false;
}
