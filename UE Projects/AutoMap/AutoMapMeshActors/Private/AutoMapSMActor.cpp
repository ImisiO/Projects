


#include "AutoMapSMActor.h"
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

#define LOCTEXT_NAMESPACE "AutoMapStaticMeshActor"

// Public Functions --------------------------------------------------------------------------------------------

AAutoMapSMActor::AAutoMapSMActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetCanBeDamaged(false);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh Component"));
	StaticMeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	StaticMeshComponent->Mobility = EComponentMobility::Static;
	StaticMeshComponent->SetGenerateOverlapEvents(false);
	StaticMeshComponent->bUseDefaultCollision = true;

	MapTagComponent = CreateDefaultSubobject<UAutoMapTagComponent>(TEXT("Auto Map Tag Component"));

	RootComponent = StaticMeshComponent;
}

void AAutoMapSMActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAutoMapSMActor::SetMobility(EComponentMobility::Type InMobility)
{
	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetMobility(InMobility);
	}
}

const FGameplayTag& AAutoMapSMActor::GetMeshCombineTag()
{
	return MapTagComponent->MapMeshCombineTag;
}

const FGameplayTag& AAutoMapSMActor::GetMapZoneTag()
{
	return MapTagComponent->MapZoneTag;
}

#if WITH_EDITOR

bool AAutoMapSMActor::GetReferencedContentObjects(TArray<UObject*>& Objects) const
{
	Super::GetReferencedContentObjects(Objects);

	if (StaticMeshComponent && StaticMeshComponent->GetStaticMesh())
	{
		Objects.Add(StaticMeshComponent->GetStaticMesh());
	}
	return true;
}

void AAutoMapSMActor::PostLoad()
{
	Super::PostLoad();

	if (StaticMeshComponent && GetLinkerCustomVersion(FFrameworkObjectVersion::GUID) < FFrameworkObjectVersion::UseBodySetupCollisionProfile)
	{
		//For all existing content we check whether we need to mark collision profile for override.
		if (UBodySetup* BodySetup = StaticMeshComponent->GetBodySetup())
		{
			if (BodySetup->DefaultInstance.GetCollisionProfileName() != StaticMeshComponent->GetCollisionProfileName())
			{
				StaticMeshComponent->bUseDefaultCollision = false;
			}
		}
	}
}

#endif // WITH_EDITOR

#if WITH_EDITOR
AAutoMapSMActor::FOnAutoMapMeshTagChanged AAutoMapSMActor::OnAutoMapMeshTagChanged;

void AAutoMapSMActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	static const FName MapTagComponentName = GET_MEMBER_NAME_CHECKED(AAutoMapSMActor, MapTagComponent);

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

//--------------------------------------------------------------------------------------------

// Protected Functions --------------------------------------------------------------------------------------------

void AAutoMapSMActor::BeginPlay()
{
	Super::BeginPlay();
}

FString AAutoMapSMActor::GetDetailedInfoInternal() const
{
	return StaticMeshComponent ? StaticMeshComponent->GetDetailedInfoInternal() : TEXT("No_StaticMeshComponent");
}

//--------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE