// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectiveMarkerSpinComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AUTOMAP3D_API UObjectiveMarkerSpinComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UObjectiveMarkerSpinComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Category = "Objective Marker Behaviours|Rotation")
	const float& GetRotationSpeed();

	UFUNCTION(Category = "Objective Marker Behaviours|Rotation")
	void SetRotationSpeed(float InSpinSpeed);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rotation")
	float RotationSpeed{0.0f};
};
