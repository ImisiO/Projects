#include "Actors/MapProxyActor.h"
#include "Subsystems/Map3DGameInstanceSubsystem.h"

// Public Functions --------------------------------------------------------------------------------------------

AMapProxyActor::AMapProxyActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	SetRootComponent(RootComponent);

	MapProxyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Map Proxy Mesh"));
	MapProxyMesh->bCastStaticShadow = false;
	MapProxyMesh->SetCastShadow(false);
	MapProxyMesh->SetVisibleInSceneCaptureOnly(true);
	MapProxyMesh->SetEnableGravity(false);
	MapProxyMesh->SetSimulatePhysics(false);
	MapProxyMesh->SetCollisionProfileName(FName(TEXT("NoCollision")));
	MapProxyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MapProxyMesh->SetupAttachment(RootComponent);
}

void AMapProxyActor::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AMapProxyActor::Tick)

		Super::Tick(DeltaTime);

	if (bShouldUpdateTransform)
	{
		UGameInstance* GameInstance = GetGameInstance();

		if (GameInstance)
		{
			UMap3DGameInstanceSubsystem* Map3DGISubSystem = GameInstance->GetSubsystem<UMap3DGameInstanceSubsystem>();

			if (Map3DGISubSystem)
			{
				const AActor* ParentActor = Map3DGISubSystem->GetProxyParent(this);

				if (ParentActor)
				{
					FRotator ParentActorRotation = ParentActor->GetActorRotation();
					FVector ParentActorLocation = ParentActor->GetActorLocation();

					FVector NewLocation;
					Map3DGISubSystem->CalculateMapWorldPosition(ParentActorLocation, NewLocation);

					SetActorLocation(NewLocation);
					SetActorRotation(ParentActorRotation);
				}
			}
		}
	}
}

bool AMapProxyActor::SetMapProxyMesh(UStaticMesh* InMesh)
{
	if (InMesh)
	{
		bool bHasSuccesfullyAddedMesh = MapProxyMesh->SetStaticMesh(InMesh);

		if (bHasSuccesfullyAddedMesh)
		{
			int32 NumberOfLods = MapProxyMesh->GetStaticMesh()->GetNumLODs();

			if (NumberOfLods > 1)
			{
				MapProxyMesh->SetForcedLodModel(NumberOfLods - 1);
			}
		}

		return bHasSuccesfullyAddedMesh;
	}

	return false;
}

bool AMapProxyActor::SetMapProxyMaterial(UMaterialInterface* InMaterial)
{
	if (InMaterial)
	{
		if (MapProxyMesh->GetStaticMesh())
		{
			int32 NumberOfMaterials = MapProxyMesh->GetNumMaterials();

			for (int32 index = 0; index < NumberOfMaterials; index++)
			{
				MapProxyMesh->SetMaterial(index, InMaterial);
			}

			return true;
		}
	}

	return false;
}

void AMapProxyActor::SetShouldUpdateTransform(bool bInShouldUpdateTransform)
{
	bShouldUpdateTransform = bInShouldUpdateTransform;
}

void AMapProxyActor::GetZoneTag(FGameplayTag& OutGameplayTag)
{
	OutGameplayTag = ZoneTag;
}

void AMapProxyActor::SetZoneTag(const FGameplayTag& InGameplayTag)
{
	ZoneTag = InGameplayTag;
}

//--------------------------------------------------------------------------------------------

// Protected Functions --------------------------------------------------------------------------------------------

void AMapProxyActor::BeginPlay()
{
	Super::BeginPlay();
	
}

//--------------------------------------------------------------------------------------------
