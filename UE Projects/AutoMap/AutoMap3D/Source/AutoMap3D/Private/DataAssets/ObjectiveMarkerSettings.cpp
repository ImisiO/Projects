#include "DataAssets/ObjectiveMarkerSettings.h"
#include "Actors/ObjectiveMapMarker.h"

// Public Functions --------------------------------------------------------------------------------------------

UStaticMesh* UObjectiveMarkerSettings::GetObjectiveMarkerMesh(EObjectiveMapMarkerType InMarkerType)
{
	if (!ObjectiveMarkerTypeToMesh.IsEmpty())
	{
		if(ObjectiveMarkerTypeToMesh.Contains(InMarkerType))
		{
			return *ObjectiveMarkerTypeToMesh.Find(InMarkerType);
		}
	}

	return nullptr;
}

EObjectiveMapMarkerDeletionType UObjectiveMarkerSettings::GetObjectiveMarkerDeletionType(EObjectiveMapMarkerType InMarkerType)
{
	if (!ObjectiveMarkerTypeToDeletionType.IsEmpty())
	{
		if (ObjectiveMarkerTypeToDeletionType.Contains(InMarkerType))
		{
			return *ObjectiveMarkerTypeToDeletionType.Find(InMarkerType);
		}
	}

	return EObjectiveMapMarkerDeletionType::Del_Null;
}

void UObjectiveMarkerSettings::GetObjectiveMarkerBehaviourSettings(EObjectiveMapMarkerType InMarkerType, FObjectiveMarkerBehaviourSettings& OutSettings)
{
	if (!ObjectiveMarkerTypeToBehaviourStack.IsEmpty())
	{
		if (ObjectiveMarkerTypeToBehaviourStack.Contains(InMarkerType))
		{
			OutSettings = *ObjectiveMarkerTypeToBehaviourStack.Find(InMarkerType);
		}
	}
}

float UObjectiveMarkerSettings::GetObjectiveMarkerDeletionDistance()
{
	return ObjectMarkerDeleteDistance;
}

//--------------------------------------------------------------------------------------------