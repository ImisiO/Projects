#include "Actors/MapViewer.h"
#include "Subsystems/Map3DGameInstanceSubsystem.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Actors/MapArea.h"
#include "Actors/MapMarkerBase.h"

// Public Functions --------------------------------------------------------------------------------------------

AMapViewer::AMapViewer()
{
	PrimaryActorTick.bCanEverTick = true;

	MapViewSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Map View Scene Component"));
	SetRootComponent(MapViewSceneComponent);

	MapCameraView = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Map View Capture"));
	
	MapCameraView->FOVAngle = DefaultFieldOfView;
	MapCameraView->ProjectionType = ECameraProjectionMode::Type::Perspective;
	MapCameraView->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	MapCameraView->CompositeMode = ESceneCaptureCompositeMode::SCCM_Overwrite;
	MapCameraView->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	MapCameraView->ShowFlags.SetAtmosphere(false);
	MapCameraView->bCaptureEveryFrame = false;
	MapCameraView->bCaptureOnMovement = false;
	MapCameraView->bAlwaysPersistRenderingState = true;
	MapCameraView->ProfilingEventName = FString(TEXT("MapView"));

	MapCameraView->SetupAttachment(MapViewSceneComponent);
}

// Called every frame
void AMapViewer::Tick(float DeltaTime)
{	
	Super::Tick(DeltaTime);

	// If the map is not open
	if(!bIsMapOpen)
	{
		FollowPlayer();
	}
	// Otherwise map is open
	else
	{
		CaptureScene();

		if(MapViewerState == EMapViewerState::Focusing)
		{
			HandleFocusingPositionBehaviour();
		}
	}
}

void AMapViewer::ApplyZoom(ECameraZoomType ZoomType)
{
	if (MapViewerState == EMapViewerState::Focusing)
	{
		return;
	}

	switch (ZoomType)
	{
		case ECameraZoomType::Zoom_Out:
		{
			// Vector pointing from the camera's focus position to the 'camera'
			FVector ToCameraPosition = GetActorLocation() - CameraFocusPosition;
			FVector ToCameraPositionNorm = ToCameraPosition.GetSafeNormal();

			// Getting new location
			FVector NewLocation = GetActorLocation() + (ToCameraPositionNorm * ZoomSpeed);

			// Checking if new zoom distance is within bounds
			float Distance = (NewLocation - CameraFocusPosition).Length();

			if (Distance < GetMaxZoomDistance())
			{
				if(ZoomCurve)
				{
					// Get current zoom ratio based on how zoomed we are compared to the min and max zoom
					float CurrentZoomRatio = GetZoomRatio(Distance);

					// Based on our ratio get our multiplier value
					float Multiplier = ZoomCurve->GetFloatValue(CurrentZoomRatio);

					// Invert multiplier when zooming out
					Multiplier = 1.0f - Multiplier;

					SetActorLocation(FMath::VInterpTo(GetActorLocation(), NewLocation, GetWorld()->GetDeltaSeconds(), Multiplier * ZoomCurveMagnitude));
				}
				//Otherwise
				else
				{
					SetActorLocation(NewLocation);
				}
			}

			break;
		}

		case ECameraZoomType::Zoom_In:
		{		
			// Vector pointing from the 'camera' to the camera's focus position
			FVector ToCameraFocusPosition = CameraFocusPosition - GetActorLocation();
			FVector ToCameraFocusPositionNorm = ToCameraFocusPosition.GetSafeNormal();

			// Getting new location
			FVector NewLocation = GetActorLocation() + (ToCameraFocusPositionNorm * ZoomSpeed);

			// Checking if new zoom distance is within bounds
			float Distance = (CameraFocusPosition - NewLocation).Length();

			if (Distance > MinZoomDistance)
			{
				if (ZoomCurve)
				{
					// Get current zoom ratio based on how zoomed we are compared to the min and max zoom
					float CurrentZoomRatio =  GetZoomRatio(Distance);

					// Based on our ratio get our multiplier value
					float Multiplier = ZoomCurve->GetFloatValue(CurrentZoomRatio);

					SetActorLocation(FMath::VInterpTo(GetActorLocation(), NewLocation, GetWorld()->GetDeltaSeconds(), Multiplier * ZoomCurveMagnitude));
				}
				//Otherwise
				else
				{
					SetActorLocation(NewLocation);
				}
			}

			break;
		}

		default:
		{
			break;
		}
	}
}

void AMapViewer::ApplyRotation(FVector2f Delta)
{
	if (MapViewerState == EMapViewerState::Focusing)
	{
		return;
	}
	
	AzimuthAngle += -Delta.X * RotationSpeed;
	ElevationAngle = FMath::Clamp(ElevationAngle + -Delta.Y * RotationSpeed, -85.0f, 85.0f);

	FVector ToFocusPosition = GetActorLocation() - CameraFocusPosition;
	float DistanceToFocusPosition = ToFocusPosition.Length();

	FVector NewPosition;
	NewPosition.X = CameraFocusPosition.X + DistanceToFocusPosition * FMath::Cos(FMath::DegreesToRadians(ElevationAngle)) * FMath::Cos(FMath::DegreesToRadians(AzimuthAngle));
	NewPosition.Y = CameraFocusPosition.Y + DistanceToFocusPosition * FMath::Cos(FMath::DegreesToRadians(ElevationAngle)) * FMath::Sin(FMath::DegreesToRadians(AzimuthAngle));
	NewPosition.Z = CameraFocusPosition.Z + DistanceToFocusPosition * FMath::Sin(FMath::DegreesToRadians(ElevationAngle));

	bool bIsWithinRange = IsPositionWithRotationRange(NewPosition);

	if (bIsWithinRange)
	{
		SetActorLocation(NewPosition);
		
		// Rotate the map view to face the target/focus position
		FRotator TargetRotation = (CameraFocusPosition - GetActorLocation()).Rotation();

		// Add 180 degrees to flip the camera upside up
		TargetRotation.Roll += 180.0f;

		SetActorRotation(TargetRotation);
	}
}	

void AMapViewer::ApplyPanXY(FVector2f Delta)
{
	if (MapViewerState == EMapViewerState::Focusing)
	{
		return;
	}
	
	float DeltaLength = Delta.Length();

	float DeltaX = (Delta.X / DeltaLength);
	float DeltaY = (Delta.Y / DeltaLength);

	FRotator CamaraRotation = GetActorRotation();

	FVector CameraForward = GetActorForwardVector().GetSafeNormal2D();
	CameraForward *= DeltaY * PanningSpeedXY;
	
	FVector CameraRight = GetActorRightVector().GetSafeNormal2D();
	CameraRight *= -DeltaX * PanningSpeedXY;

	CameraFocusPosition += CameraForward + CameraRight;

	FVector NewLocation = GetActorLocation() + CameraForward + CameraRight;
	SetActorLocation(NewLocation);
}

void AMapViewer::ApplyPanUp()
{
	FVector UpVector = FVector::UpVector * PanningSpeedZ;

	CameraFocusPosition += UpVector;

	FVector NewLocation = GetActorLocation() + UpVector;
	SetActorLocation(NewLocation);
}

void AMapViewer::ApplyPanDown()
{
	FVector DownVector = FVector::DownVector * PanningSpeedZ;

	CameraFocusPosition += DownVector;

	FVector NewLocation = GetActorLocation() + DownVector;
	SetActorLocation(NewLocation);
}

void AMapViewer::FocusPlayer()
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);

	if (PlayerCharacter)
	{	
		FVector NewFocusPosition{}; 
		
		GetPlayerMarkerPosition(NewFocusPosition);

		FVector CameraTargetLocation = NewFocusPosition + PlayerCharacter->GetActorUpVector() * PlayerFollowUpwardsOffset;
		CameraTargetLocation -= PlayerCharacter->GetActorForwardVector() * PlayerFollowBackwardsOffset;

		FocusPosition(NewFocusPosition, CameraTargetLocation);
	}
}
void AMapViewer::OpenMap()
{
	bIsMapOpen = true;
}

void AMapViewer::CloseMap()
{
	bIsMapOpen = false;
}

void AMapViewer::SetFOV(float InFOV)
{
	if(MapCameraView)
	{
		MapCameraView->FOVAngle = InFOV;
	}
}

float AMapViewer::GetFOV()
{
	if (MapCameraView)
	{
		return MapCameraView->FOVAngle;
	}

	return 0.0f;
}

float AMapViewer::GetMaxFOV()
{
	return MaxFieldOfView;
}

void AMapViewer::SetPlayerFollowUpwardsOffset(float InOffset)
{
	PlayerFollowUpwardsOffset = InOffset;
	CalculateNewDefaultZoomDistance();
	CalculateRotationAngles();
}

float AMapViewer::GetPlayerFollowUpwardsOffset()
{
	return PlayerFollowUpwardsOffset;
}

void AMapViewer::SetPlayerFollowBackwardsOffset(float InOffset)
{
	PlayerFollowBackwardsOffset = InOffset;
	CalculateNewDefaultZoomDistance();
	CalculateRotationAngles();
}

float AMapViewer::GetPlayerFollowBackwardsOffset()
{
	return PlayerFollowBackwardsOffset;
}

void AMapViewer::SetZoomDistanceOffset(float InOffset)
{
	ZoomDistanceOffset = InOffset;
}

float AMapViewer::GetZoomDistanceOffset()
{
	return ZoomDistanceOffset;
}

float AMapViewer::GetMaxZoomDistance()
{
	return DefaultZoomDistance + ZoomDistanceOffset;
}

void AMapViewer::AddObjectToMapView(AActor* InActor)
{
	if (InActor)
	{
		if(MapCameraView)
		{
			MapCameraView->ShowOnlyActors.AddUnique(InActor);
		}
	}
}

void AMapViewer::AddComponentToMapView(UPrimitiveComponent* InComponent)
{
	if(InComponent)
	{
		MapCameraView->ShowOnlyComponent(InComponent);
	}
}

void AMapViewer::RemoveObjectFromMapView(AActor* InActor)
{
	if (InActor)
	{
		if (MapCameraView)
		{
			bool bIsObjectInMapView = MapCameraView->ShowOnlyActors.Contains(InActor);

			if(bIsObjectInMapView)
			{
				MapCameraView->ShowOnlyActors.RemoveSingle(InActor);
			}
		}
	}
}

void AMapViewer::ResizeViewerRenderTarget(int32 InWidth, int32 InHeight)
{
	if(MapCameraView->TextureTarget)
	{
		UKismetRenderingLibrary::ResizeRenderTarget2D(MapCameraView->TextureTarget, InWidth, InHeight);
	}
}

//--------------------------------------------------------------------------------------------

// Protected Functions --------------------------------------------------------------------------------------------

void AMapViewer::BeginPlay()
{
	Super::BeginPlay();
	
	SetFOV(DefaultFieldOfView);
}

void AMapViewer::FollowPlayer()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AMapViewer::FollowPlayer);

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);

	if (PlayerCharacter)
	{
		FVector PlayerMarkerLocation{};
		
		// Getting player's position relative to the map area
		GetPlayerMarkerPosition(PlayerMarkerLocation);

		// Calculate the offset position above the and behind the player 
		FVector CameraTargetLocation = PlayerMarkerLocation + PlayerCharacter->GetActorUpVector() * PlayerFollowUpwardsOffset;
		CameraTargetLocation -= PlayerCharacter->GetActorForwardVector() * PlayerFollowBackwardsOffset;

		SetActorLocation(FMath::VInterpTo(GetActorLocation(), CameraTargetLocation, GetWorld()->GetDeltaSeconds(), PlayerFollowMovementLerpSpeed));

		// Rotate the map view to face the player
		FRotator TargetRotation = (PlayerMarkerLocation - GetActorLocation()).Rotation();
		
		// Add 180 degrees to flip the camera upside up
		TargetRotation.Roll += 180.0f;

		SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), PlayerFollowRotateLerpSpeed));

		CameraFocusPosition = PlayerMarkerLocation;

		CalculateNewDefaultZoomDistance();
		CalculateRotationAngles();
	}
}

void AMapViewer::FocusPosition(const FVector& PositonToFocus, const FVector& CameraTargetPosition)
{
	MapViewerState = EMapViewerState::Focusing;
	CameraFocusPosition = PositonToFocus;
	NewCameraPosition = CameraTargetPosition;
}

void AMapViewer::InterruptFocusingState()
{
	MapViewerState = EMapViewerState::Default;
}

void AMapViewer::HandleFocusingPositionBehaviour()
{
	if(HasFinishedFocusing())
	{
		CalculateRotationAngles();
		InterruptFocusingState();
	} 
	else
	{
		SetActorLocation(FMath::VInterpTo(GetActorLocation(), NewCameraPosition, GetWorld()->GetDeltaSeconds(), FocusPointLerpSpeed));
		FRotator TargetRotation = (CameraFocusPosition - GetActorLocation()).Rotation();
		TargetRotation.Roll += 180.0f;
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), FocusPointLerpSpeed));
	}
}

bool AMapViewer::HasFinishedFocusing()
{
	float Tolerance = 0.1f;
	FVector CurrentCameraPosition = GetActorLocation();

	float DiffX = CurrentCameraPosition.X - NewCameraPosition.X;
	float DiffY = CurrentCameraPosition.Y - NewCameraPosition.Y;
	float DiffZ = CurrentCameraPosition.Z - NewCameraPosition.Z;

	bool bIsXInTolerance = DiffX >= 0.0f && DiffX < Tolerance;
	bool bIsYInTolerance = DiffY >= 0.0f && DiffY < Tolerance;
	bool bIsZInTolerance = DiffZ >= 0.0f && DiffZ < Tolerance;

	return bIsXInTolerance && bIsYInTolerance && bIsZInTolerance;
}

void AMapViewer::GetPlayerMarkerPosition(FVector& OutPlayerPosition)
{
	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance)
	{
		UMap3DGameInstanceSubsystem* Map3DGISubSystem = GameInstance->GetSubsystem<UMap3DGameInstanceSubsystem>();

		if (Map3DGISubSystem)
		{
			Map3DGISubSystem->GetPlayerMarkerPosition(OutPlayerPosition);
		}
	}
}

float AMapViewer::GetZoomRatio(float CurrentZoomDistance)
{
	float Ratio = (CurrentZoomDistance - MinZoomDistance) / (GetMaxZoomDistance() - MinZoomDistance);

	Ratio = FMath::Clamp(Ratio, 0.0f, 1.0f);

	return Ratio;
}

bool AMapViewer::IsPositionWithRotationRange(const FVector& NewLocation)
{
	FVector VectorA = (NewLocation - CameraFocusPosition).GetSafeNormal();
	FVector VectorB = FVector::UpVector.GetSafeNormal();

	double DotProduct = FVector::DotProduct(VectorA, VectorB);

	return DotProduct > -0.8 && DotProduct < 0.99;
}

void AMapViewer::CalculateRotationAngles()
{
	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance)
	{
		UMap3DGameInstanceSubsystem* Map3DGISubSystem = GameInstance->GetSubsystem<UMap3DGameInstanceSubsystem>();

		if (Map3DGISubSystem)
		{
			FRotator PlayerMarkerRotation;
			Map3DGISubSystem->GetPlayerMarkerRotation(PlayerMarkerRotation);
			FVector InitialOffset = FVector(-PlayerFollowBackwardsOffset, 0.0f, PlayerFollowUpwardsOffset);
			InitialOffset = PlayerMarkerRotation.RotateVector(InitialOffset);
			FVector Direction = InitialOffset.GetSafeNormal();

			ElevationAngle = FMath::RadiansToDegrees(FMath::Asin(Direction.Z));
			AzimuthAngle = FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
		}
	}
}

void AMapViewer::CalculateNewDefaultZoomDistance()
{
	FVector2f CameraOffset = FVector2f{ PlayerFollowBackwardsOffset , PlayerFollowUpwardsOffset };
	DefaultZoomDistance = FMath::Floor(CameraOffset.Length());
}

void AMapViewer::CaptureScene()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AMapViewer::CaptureScene);
	MapCameraView->CaptureSceneDeferred();
}

//--------------------------------------------------------------------------------------------