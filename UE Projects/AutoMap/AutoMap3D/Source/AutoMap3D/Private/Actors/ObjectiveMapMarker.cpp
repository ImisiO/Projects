#include "Actors/ObjectiveMapMarker.h"
#include "Subsystems/Map3DGameInstanceSubsystem.h"

// Public Functions --------------------------------------------------------------------------------------------

void AObjectiveMapMarker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleDeletionBehaviour();
}

bool AObjectiveMapMarker::SetMesh(UStaticMesh* InStaticMesh)
{
	if (InStaticMesh)
	{
		bool bHasSuccessfullyAddedMesh = MarkerMesh->SetStaticMesh(InStaticMesh);

		if (!bIsMarkerMeshVisibleInSceneCapture && bIsMapWorldObjectiveMarker)
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

void AObjectiveMapMarker::SetMarkerDeletionType(EObjectiveMapMarkerDeletionType InDeletionType)
{
	MarkerDeletionType = InDeletionType;
}

void AObjectiveMapMarker::GetMarkerDeletionType(EObjectiveMapMarkerDeletionType& OutDeletionType)
{
	OutDeletionType = MarkerDeletionType;
}

void AObjectiveMapMarker::SetIsObjectiveMarkerInMapWorld(bool bIsInMapWorld)
{
	bIsMapWorldObjectiveMarker = bIsInMapWorld;
}

bool AObjectiveMapMarker::IsObjectMarkerInMapWorld()
{
	return bIsMapWorldObjectiveMarker;
}

//--------------------------------------------------------------------------------------------

// Protected Functions --------------------------------------------------------------------------------------------

void AObjectiveMapMarker::BeginPlay()
{
	Super::BeginPlay();
}

void AObjectiveMapMarker::HandleDeletionBehaviour()
{
	if(bIsMapWorldObjectiveMarker)
	{
		switch (MarkerDeletionType)
		{
			case EObjectiveMapMarkerDeletionType::Del_Null:
			{
				break;
			}

			case EObjectiveMapMarkerDeletionType::Del_Distance:
			{
				UGameInstance* GameInstance = GetGameInstance();

				if (GameInstance)
				{
					UMap3DGameInstanceSubsystem* Map3DGISubSystem = GameInstance->GetSubsystem<UMap3DGameInstanceSubsystem>();

					if (Map3DGISubSystem)
					{
						FVector PlayerMarkerPosition;
						Map3DGISubSystem->GetPlayerMarkerPosition(PlayerMarkerPosition);

						FVector ToPlayerMarkerPosition = PlayerMarkerPosition - GetActorLocation();
					
						float Distance = ToPlayerMarkerPosition.Length();

						// If the player marker is within a certain range of the objective marker
						if(Distance <= Map3DGISubSystem->GetObjectiveMarkerDeleteDistance())
						{
							// Remove the objective marker from the view and delete the object
							Map3DGISubSystem->RemoveObjectiveMarker(this);
						}
					}
				}
			
				break;
			}

			default:
			{
				break;
			}
		}
	}
}

//--------------------------------------------------------------------------------------------