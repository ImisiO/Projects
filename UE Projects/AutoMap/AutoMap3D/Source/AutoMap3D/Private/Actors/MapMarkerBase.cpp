#include "Actors/MapMarkerBase.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/Map3DGameInstanceSubsystem.h"

// Public Functions --------------------------------------------------------------------------------------------

AMapMarkerBase::AMapMarkerBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	SetRootComponent(RootComponent);

	MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Map Marker Mesh"));
	MarkerMesh->bCastStaticShadow = false;
	MarkerMesh->SetCastShadow(false);
	MarkerMesh->SetVisibleInSceneCaptureOnly(bIsMarkerMeshVisibleInSceneCapture); // Setting to false initially as you would need to line it up in the editor
	MarkerMesh->SetEnableGravity(false);
	MarkerMesh->SetSimulatePhysics(false);
	MarkerMesh->SetCollisionProfileName(FName(TEXT("NoCollision")));
	MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MarkerMesh->SetupAttachment(RootComponent);
}

void AMapMarkerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AMapMarkerBase::SetMesh(UStaticMesh* InStaticMesh)
{
	if (InStaticMesh)
	{
		bool bHasSuccessfullyAddedMesh = MarkerMesh->SetStaticMesh(InStaticMesh);

		if (!bIsMarkerMeshVisibleInSceneCapture)
		{
			ToggleMeshSceneCaptureVisibility();
		}

		if (bHasSuccessfullyAddedMesh)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------------------

// Protected Functions --------------------------------------------------------------------------------------------

void AMapMarkerBase::BeginPlay()
{
	Super::BeginPlay();

	if (MarkerMesh)
	{
		if (MarkerMesh->GetStaticMesh())
		{
			MarkerMesh->SetVisibleInSceneCaptureOnly(true);
			bIsMarkerMeshVisibleInSceneCapture = true;
		}
	}
}

void AMapMarkerBase::ToggleMeshSceneCaptureVisibility()
{
	if (MarkerMesh)
	{
		if (MarkerMesh->GetStaticMesh())
		{
			MarkerMesh->SetVisibleInSceneCaptureOnly(!bIsMarkerMeshVisibleInSceneCapture);
			bIsMarkerMeshVisibleInSceneCapture = !bIsMarkerMeshVisibleInSceneCapture;
		}
	}
}

void AMapMarkerBase::GetMapPosition(const FVector& InVector, FVector& OutVector)
{
	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance)
	{
		UMap3DGameInstanceSubsystem* Map3DGISubSystem = GameInstance->GetSubsystem<UMap3DGameInstanceSubsystem>();

		if (Map3DGISubSystem)
		{
			Map3DGISubSystem->CalculateMapWorldPosition(InVector, OutVector);
		}
	}
}

//--------------------------------------------------------------------------------------------