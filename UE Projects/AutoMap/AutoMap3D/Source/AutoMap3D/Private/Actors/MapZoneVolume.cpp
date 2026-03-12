#include "Actors/MapZoneVolume.h"
#include "Actors/PlayerMapMarker.h"
#include "Subsystems/Map3DGameInstanceSubsystem.h"

// Public Functions --------------------------------------------------------------------------------------------

AMapZoneVolume::AMapZoneVolume()
{
	PrimaryActorTick.bCanEverTick = true;

	ZoneVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Zone Volume"));
	ZoneVolume->SetLineThickness(15.0f);
	ZoneVolume->SetCanEverAffectNavigation(false);
	ZoneVolume->OnComponentBeginOverlap.AddDynamic(this, &AMapZoneVolume::ActivateMap);
	SetRootComponent(ZoneVolume);
}

void AMapZoneVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//--------------------------------------------------------------------------------------------

// Protected Functions --------------------------------------------------------------------------------------------

void AMapZoneVolume::BeginPlay()
{
	Super::BeginPlay();
}

void AMapZoneVolume::ActivateMap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
 	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance && !IsAssociatedMapAreaActive())
	{
		UMap3DGameInstanceSubsystem* Map3DGISubSystem = GameInstance->GetSubsystem<UMap3DGameInstanceSubsystem>();

		if (Map3DGISubSystem)
		{
			if (OtherActor)
			{
				APlayerMapMarker* PlayerMarker = Cast<APlayerMapMarker>(OtherActor);

				if (PlayerMarker)
				{
					Map3DGISubSystem->ActivateMapArea(ZoneTag);
				}
			}
		}
	}
}

bool AMapZoneVolume::IsAssociatedMapAreaActive()
{
	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance)
	{
		UMap3DGameInstanceSubsystem* Map3DGISubSystem = GameInstance->GetSubsystem<UMap3DGameInstanceSubsystem>();

		if (Map3DGISubSystem)
		{
			return Map3DGISubSystem->IsMapAreaActive(ZoneTag);
		}
		else
		{
			return false;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------------------