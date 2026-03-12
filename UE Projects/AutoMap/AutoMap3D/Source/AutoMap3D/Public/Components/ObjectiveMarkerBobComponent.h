#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectiveMarkerBobComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AUTOMAP3D_API UObjectiveMarkerBobComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UObjectiveMarkerBobComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	const FVector& GetOwnerSpawnPosition();

	void SetOwnerSpawnPosition(const FVector& InVector);

	const float& GetBobLength();

	void SetBobLength(float InLength);

	const float& GetBobSpeed();

	void SetBobSpeed(float InSpeed);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Objective Marker Behaviours|Bob")
	float BobTimer{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Objective Marker Behaviours|Bob")
	float BobSpeed{2.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Objective Marker Behaviours|Bob")
	float BobLength{2.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Objective Marker Behaviours|Bob")
	FVector OwnerSpawnPosition{};
};
