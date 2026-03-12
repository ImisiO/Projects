#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "AI/Navigation/NavigationTypes.h"
#include "AutoMapTagComponent.h"
#include "AutoMapSMActor.generated.h"

UCLASS()
class AUTOMAPMESHACTORS_API AAutoMapSMActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAutoMapSMActor();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = Mobility)
	virtual void SetMobility(EComponentMobility::Type InMobility);

	const FGameplayTag& GetMeshCombineTag();

	const FGameplayTag& GetMapZoneTag();

	virtual ENavDataGatheringMode GetGeometryGatheringMode() const { return ENavDataGatheringMode::Default; }

	class UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }

#if WITH_EDITOR
	virtual bool GetReferencedContentObjects(TArray<UObject*>& Objects) const override;
	virtual void PostLoad() override;
#endif // WITH_EDITOR

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAutoMapMeshTagChanged, AAutoMapSMActor*);
	static FOnAutoMapMeshTagChanged OnAutoMapMeshTagChanged;
#endif // WITH_EDITOR

protected:
	virtual void BeginPlay() override;

	virtual FString GetDetailedInfoInternal() const override;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMeshComponent* StaticMeshComponent{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Auto Map Tags")
	UAutoMapTagComponent* MapTagComponent{ nullptr };

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Actor")
	bool bStaticMeshReplicateMovement = false;
};