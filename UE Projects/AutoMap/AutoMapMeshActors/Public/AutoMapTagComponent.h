#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AutoMapTagComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AUTOMAPMESHACTORS_API UAutoMapTagComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAutoMapTagComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Gameplay Tags|Map Zone Tag")
	FGameplayTag MapZoneTag;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Gameplay Tags|Map Mesh Combine Tag")
	FGameplayTag MapMeshCombineTag;

protected:
	virtual void BeginPlay() override;
};
