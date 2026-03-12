

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "AI/Navigation/NavigationTypes.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "AutoMapTagComponent.h"
#include "AutoMapHISMActor.generated.h"

UCLASS()
class AUTOMAPMESHACTORS_API AAutoMapHISMActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAutoMapHISMActor();

	virtual void Tick(float DeltaTime) override;

	const FGameplayTag& GetMeshCombineTag();

	const FGameplayTag& GetMapZoneTag();

	UFUNCTION(BlueprintCallable, Category = Mobility)
	virtual void SetMobility(EComponentMobility::Type InMobility);

#if WITH_EDITOR
	virtual bool GetReferencedContentObjects(TArray<UObject*>& Objects) const override;
	virtual void PostLoad() override;
#endif // WITH_EDITOR

	virtual ENavDataGatheringMode GetGeometryGatheringMode() const { return ENavDataGatheringMode::Default; }

	class UHierarchicalInstancedStaticMeshComponent* GetStaticMeshComponent() const { return HierarchicalInstancedStaticMeshComponent; }

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAutoMapMeshTagChanged, AAutoMapHISMActor*);
	static FOnAutoMapMeshTagChanged OnAutoMapMeshTagChanged;
#endif // WITH_EDITOR

protected:
	virtual void BeginPlay() override;

	virtual FString GetDetailedInfoInternal() const override;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UHierarchicalInstancedStaticMeshComponent* HierarchicalInstancedStaticMeshComponent{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Auto Map Tags")
	UAutoMapTagComponent* MapTagComponent{ nullptr };

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Actor")
	bool bStaticMeshReplicateMovement;
};