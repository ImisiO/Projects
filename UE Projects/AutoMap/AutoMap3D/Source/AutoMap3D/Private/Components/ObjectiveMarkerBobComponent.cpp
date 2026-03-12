#include "Components/ObjectiveMarkerBobComponent.h"

// Public Functions --------------------------------------------------------------------------------------------

UObjectiveMarkerBobComponent::UObjectiveMarkerBobComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bAutoRegister = true;
}

void UObjectiveMarkerBobComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UObjectiveMarkerBobComponent::TickComponent)

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();

	if(Owner)
	{
		BobTimer += DeltaTime; 
		float ZOffset = BobLength * sin(BobSpeed * BobTimer);
		
		FVector NewPosition = OwnerSpawnPosition;
		NewPosition.Z += ZOffset;

		Owner->SetActorLocation(NewPosition);
	}
}

const FVector& UObjectiveMarkerBobComponent::GetOwnerSpawnPosition()
{
	return OwnerSpawnPosition;
}

void UObjectiveMarkerBobComponent::SetOwnerSpawnPosition(const FVector& InVector)
{
	OwnerSpawnPosition = InVector;
}

const float& UObjectiveMarkerBobComponent::GetBobLength()
{
	return BobLength;
}

void UObjectiveMarkerBobComponent::SetBobLength(float InLength)
{
	BobLength = InLength;
}

const float& UObjectiveMarkerBobComponent::GetBobSpeed()
{
	return BobSpeed;
}

void UObjectiveMarkerBobComponent::SetBobSpeed(float InSpeed)
{
	BobSpeed = InSpeed;
}

//--------------------------------------------------------------------------------------------

// Protected Functions --------------------------------------------------------------------------------------------

void UObjectiveMarkerBobComponent::BeginPlay()
{
	Super::BeginPlay();
}

//--------------------------------------------------------------------------------------------