#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ObjectiveMarkerSettings.generated.h"

enum class EObjectiveMapMarkerType : uint8;
enum class EObjectiveMapMarkerDeletionType : uint8; 
enum class EObjectiveMapMarkerBehaviour : uint8;

USTRUCT(BlueprintType)
struct FObjectiveMarkerBehaviourSettings
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<EObjectiveMapMarkerBehaviour> BehaviourStack{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Marker Behaviour Settings|Spin Behaviour")
	float RotationSpeed{2.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Marker Behaviour Settings|Bob Behaviour")
	float BobSpeed{2.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Marker Behaviour Settings|Bob Behaviour")
	float BobLength{2.0f};
};

UCLASS()
class AUTOMAP3D_API UObjectiveMarkerSettings : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UStaticMesh* GetObjectiveMarkerMesh(EObjectiveMapMarkerType InMarkerType);
	
	EObjectiveMapMarkerDeletionType GetObjectiveMarkerDeletionType(EObjectiveMapMarkerType InMarkerType);
	
	void GetObjectiveMarkerBehaviourSettings(EObjectiveMapMarkerType InMarkerType, FObjectiveMarkerBehaviourSettings& OutSettings);
	
	float GetObjectiveMarkerDeletionDistance();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Marker Mesh Settings")
	TMap<EObjectiveMapMarkerType, UStaticMesh*> ObjectiveMarkerTypeToMesh{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Marker Deletion Settings")
	TMap<EObjectiveMapMarkerType, EObjectiveMapMarkerDeletionType> ObjectiveMarkerTypeToDeletionType{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Marker Behaviour Settings")
	TMap<EObjectiveMapMarkerType, FObjectiveMarkerBehaviourSettings> ObjectiveMarkerTypeToBehaviourStack{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Marker Deletion Settings")
	float ObjectMarkerDeleteDistance{50.0f};
};
