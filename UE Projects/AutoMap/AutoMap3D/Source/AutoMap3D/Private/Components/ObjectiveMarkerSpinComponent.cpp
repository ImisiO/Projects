#include "Components/ObjectiveMarkerSpinComponent.h"

// Public Functions --------------------------------------------------------------------------------------------

UObjectiveMarkerSpinComponent::UObjectiveMarkerSpinComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bAutoRegister = true;
}

void UObjectiveMarkerSpinComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UObjectiveMarkerSpinComponent::TickComponent)

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();

	if (Owner)
	{
		FRotator RotationAmount = FRotator(0.0f, RotationSpeed, 0.0f);
		Owner->AddActorLocalRotation(RotationAmount.Quaternion());
	}
}

const float& UObjectiveMarkerSpinComponent::GetRotationSpeed()
{
	return RotationSpeed;
}

void UObjectiveMarkerSpinComponent::SetRotationSpeed(float InSpinSpeed)
{
	RotationSpeed = InSpinSpeed;
}

//--------------------------------------------------------------------------------------------

// Protected Functions --------------------------------------------------------------------------------------------

void UObjectiveMarkerSpinComponent::BeginPlay()
{
	Super::BeginPlay();
}

//--------------------------------------------------------------------------------------------