#include "Components/BulletTimeComponent.h"

UBulletTimeComponent::UBulletTimeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UBulletTimeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* pThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, pThisTickFunction);
}

void UBulletTimeComponent::ApplyPinnedDilation()
{
	if (PinnedDilationFactor < 0.0f) 
	{
		return;
	}

	if (AActor* pOwner = GetOwner())
	{
		pOwner->CustomTimeDilation = PinnedDilationFactor;
	}
}

void UBulletTimeComponent::ClearPinnedDilation()
{
	PinnedDilationFactor = -1.0f;

	if (AActor* Owner = GetOwner())
	{
		Owner->CustomTimeDilation = 1.0f;
	}
}

void UBulletTimeComponent::SetPinnedDilation(float Factor)
{
	PinnedDilationFactor = Factor;
	ApplyPinnedDilation();
}

void UBulletTimeComponent::BeginPlay()
{
	Super::BeginPlay();
	ApplyPinnedDilation();
}

