#include "Actors/PlayerMapMarker.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

// Public Functions --------------------------------------------------------------------------------------------

void APlayerMapMarker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FollowPlayer();
}

//--------------------------------------------------------------------------------------------

// Protected Functions --------------------------------------------------------------------------------------------

void APlayerMapMarker::BeginPlay()
{
	Super::BeginPlay();

	MarkerMesh->SetCollisionProfileName(FName(TEXT("OverlapAll")));
	MarkerMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void APlayerMapMarker::FollowPlayer()
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);

	if (PlayerCharacter)
	{
		FVector PlayerMapLocation;
		GetMapPosition(PlayerCharacter->GetActorLocation(), PlayerMapLocation);
		FRotator PlayerRotation = PlayerCharacter->GetActorRotation();

		SetActorLocation(PlayerMapLocation);
		SetActorRotation(PlayerRotation);
	}
}

//--------------------------------------------------------------------------------------------