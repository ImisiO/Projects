


#include "AutoMapHISMActor.h"
#include "UObject/FrameworkObjectVersion.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
#include "Misc/MapErrors.h"
#include "PhysicsEngine/BodySetup.h"
#include "StaticMeshComponentLODInfo.h"
#include "StaticMeshResources.h"
#include "Engine/StaticMesh.h"
#include "UObject/UnrealType.h"

#define LOCTEXT_NAMESPACE "AutoMapHISMActor"

// Public Functions --------------------------------------------------------------------------------------------

AAutoMapHISMActor::AAutoMapHISMActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SetCanBeDamaged(false);

	HierarchicalInstancedStaticMeshComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Hierarchical Instanced Static Mesh Component"));
	HierarchicalInstancedStaticMeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	HierarchicalInstancedStaticMeshComponent->Mobility = EComponentMobility::Static;
	HierarchicalInstancedStaticMeshComponent->SetGenerateOverlapEvents(false);
	HierarchicalInstancedStaticMeshComponent->bUseDefaultCollision = true;

	MapTagComponent = CreateDefaultSubobject<UAutoMapTagComponent>(TEXT("Auto Map Tag Component"));

	RootComponent = HierarchicalInstancedStaticMeshComponent;
}

void AAutoMapHISMActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

const FGameplayTag& AAutoMapHISMActor::GetMeshCombineTag()
{
	return MapTagComponent->MapMeshCombineTag;
}

const FGameplayTag& AAutoMapHISMActor::GetMapZoneTag()
{
	return MapTagComponent->MapZoneTag;
}

void AAutoMapHISMActor::SetMobility(EComponentMobility::Type InMobility)
{
	if (HierarchicalInstancedStaticMeshComponent)
	{
		HierarchicalInstancedStaticMeshComponent->SetMobility(InMobility);
	}
}

#if WITH_EDITOR

bool AAutoMapHISMActor::GetReferencedContentObjects(TArray<UObject*>& Objects) const
{
	Super::GetReferencedContentObjects(Objects);

	if (HierarchicalInstancedStaticMeshComponent && HierarchicalInstancedStaticMeshComponent->GetStaticMesh())
	{
		Objects.Add(HierarchicalInstancedStaticMeshComponent->GetStaticMesh());
	}
	return true;
}

void AAutoMapHISMActor::PostLoad()
{
	Super::PostLoad();

	if (HierarchicalInstancedStaticMeshComponent && GetLinkerCustomVersion(FFrameworkObjectVersion::GUID) < FFrameworkObjectVersion::UseBodySetupCollisionProfile)
	{
		//For all existing content we check whether we need to mark collision profile for override.
		if (UBodySetup* BodySetup = HierarchicalInstancedStaticMeshComponent->GetBodySetup())
		{
			if (BodySetup->DefaultInstance.GetCollisionProfileName() != HierarchicalInstancedStaticMeshComponent->GetCollisionProfileName())
			{
				HierarchicalInstancedStaticMeshComponent->bUseDefaultCollision = false;
			}
		}
	}
}

#endif // WITH_EDITOR

#if WITH_EDITOR
AAutoMapHISMActor::FOnAutoMapMeshTagChanged AAutoMapHISMActor::OnAutoMapMeshTagChanged;

void AAutoMapHISMActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	static const FName MapTagComponentName = GET_MEMBER_NAME_CHECKED(AAutoMapHISMActor, MapTagComponent);

	const FName ChangedProperty = PropertyChangedEvent.GetPropertyName();
	const FName MemberChanged = PropertyChangedEvent.MemberProperty
		? PropertyChangedEvent.MemberProperty->GetFName()
		: NAME_None;

	const bool bIsTagOrTagComponentChanged = ChangedProperty == MapTagComponentName;

	if (bIsTagOrTagComponentChanged)
	{
		OnAutoMapMeshTagChanged.Broadcast(this);
	}
}
#endif // WITH_EDITOR

// Protected Functions --------------------------------------------------------------------------------------------

void AAutoMapHISMActor::BeginPlay()
{
	Super::BeginPlay();
}

FString AAutoMapHISMActor::GetDetailedInfoInternal() const
{
	return HierarchicalInstancedStaticMeshComponent ? HierarchicalInstancedStaticMeshComponent->GetDetailedInfoInternal() : TEXT("No_HierarchicalInstancedStaticMeshComponent");
}

//--------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE