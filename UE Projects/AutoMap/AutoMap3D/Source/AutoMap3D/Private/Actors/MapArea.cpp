#include "Actors/MapArea.h"
#include "Subsystems/Map3DGameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AutoMapSMActor.h"
#include "AutoMapHISMActor.h"
#include "Actors/PlayerMapMarker.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetRenderingLibrary.h"

// Public Functions --------------------------------------------------------------------------------------------

AMapArea::AMapArea()
{
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	SetRootComponent(RootComponent);

	TerrainSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Terrain Scene Comp"));
	TerrainSceneComponent->SetupAttachment(RootComponent);

	AccentSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Accent Scene Comp"));
	AccentSceneComponent->SetupAttachment(TerrainSceneComponent);

	TerrainMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Terrain Map Element Mesh"));
	TerrainMeshComp->bCastStaticShadow = false;
	TerrainMeshComp->SetCastShadow(false);
	TerrainMeshComp->SetVisibleInSceneCaptureOnly(bIsMeshOnlyVisibleInSceneCapture);
	TerrainMeshComp->SetEnableGravity(false);
	TerrainMeshComp->SetSimulatePhysics(false);
	TerrainMeshComp->SetLightingChannels(false, true, false);
	TerrainMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TerrainMeshComp->SetCollisionProfileName(FName(TEXT("NoCollision")));
	TerrainMeshComp->SetGenerateOverlapEvents(false);
	TerrainMeshComp->SetupAttachment(TerrainSceneComponent);

	TerrainUnwrapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Terrain Unwrap Mesh"));
	TerrainUnwrapMesh->bCastStaticShadow = false;
	TerrainUnwrapMesh->SetCastShadow(false);
	TerrainUnwrapMesh->SetVisibleInSceneCaptureOnly(true);
	TerrainUnwrapMesh->SetEnableGravity(false);
	TerrainUnwrapMesh->SetSimulatePhysics(false);
	TerrainUnwrapMesh->SetLightingChannels(false, true, false);
	TerrainUnwrapMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TerrainUnwrapMesh->SetCollisionProfileName(FName(TEXT("NoCollision")));
	TerrainUnwrapMesh->SetupAttachment(TerrainSceneComponent);

	AccentMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Accent Map Element Mesh"));
	AccentMeshComp->bCastStaticShadow = false;
	AccentMeshComp->SetCastShadow(false);
	AccentMeshComp->SetVisibleInSceneCaptureOnly(bIsMeshOnlyVisibleInSceneCapture);
	AccentMeshComp->SetEnableGravity(false);
	AccentMeshComp->SetSimulatePhysics(false);
	AccentMeshComp->SetLightingChannels(false, true, false);
	AccentMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AccentMeshComp->SetCollisionProfileName(FName(TEXT("NoCollision")));
	AccentMeshComp->SetupAttachment(AccentSceneComponent);

	FRotator PaintCaptureRotator{ -90.0, -180.0, 90.0 };

	TerrainPaintCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Map Terrain Paint Capture"));
	TerrainPaintCapture->ProjectionType = ECameraProjectionMode::Type::Orthographic;
	TerrainPaintCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	TerrainPaintCapture->CompositeMode = ESceneCaptureCompositeMode::SCCM_Additive;
	TerrainPaintCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	TerrainPaintCapture->ShowFlags.SetAtmosphere(false);
	TerrainPaintCapture->bCaptureEveryFrame = false;
	TerrainPaintCapture->bCaptureOnMovement = false;
	TerrainPaintCapture->bAlwaysPersistRenderingState = true;
	TerrainPaintCapture->ProfilingEventName = FString(TEXT("MapTerrainPaint"));
	TerrainPaintCapture->AddRelativeRotation(PaintCaptureRotator);
	TerrainPaintCapture->SetupAttachment(TerrainSceneComponent);
}

void AMapArea::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleViewOcclusionBehaviour();
	HandlePaintBehaviour(DeltaTime);
}

void AMapArea::OpenMap()
{
	bIsMapOpen = true;
}

void AMapArea::CloseMap()
{
	bIsMapOpen = false;
}

void AMapArea::PaintArea(const FVector& PaintLocation, float InTerrainPaintRadius)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AMapArea::PaintArea)

	if (AreMeshMaterialsValid() && AreUnwrapMaterialsValid())
	{
		// Updating unwrap material parameters
		FName PaintLocationName = FName(TEXT("PaintLocation"));
		FName PaintRadiusName = FName(TEXT("Radius"));

		DynamicTerrainUnwrapMaterial->SetVectorParameterValue(PaintLocationName, PaintLocation);
		DynamicTerrainUnwrapMaterial->SetScalarParameterValue(PaintRadiusName, InTerrainPaintRadius);

		{
			TerrainPaintCapture->CaptureScene();
		}
	}
}

void AMapArea::SetupPreBeginPlayParameters()
{
	SetMeshComponentSceneCaptureVisibility(true);
	SetupTerrainUnwrapMesh();
	SetupCaptureParameters();
}

void AMapArea::SetupMaterialsParameters()
{
	if (DynamicTerrainUnwrapMaterial)
	{
		FName PaintRadiusName = FName(TEXT("Radius"));
		FName PaintHarndessName = FName(TEXT("Hardness"));
		FName CaptureSizeName = FName(TEXT("CaptureSize"));
		FName UnwrapLocationName = FName(TEXT("UnwrapLocation"));

		DynamicTerrainUnwrapMaterial->SetScalarParameterValue(PaintRadiusName, TerrainPaintRadius);
		DynamicTerrainUnwrapMaterial->SetScalarParameterValue(PaintHarndessName, PaintHardness);
		DynamicTerrainUnwrapMaterial->SetScalarParameterValue(CaptureSizeName, TerrainPaintCapture->OrthoWidth);
		DynamicTerrainUnwrapMaterial->SetVectorParameterValue(UnwrapLocationName, UnwrapLocation);
	}

	if (DynamicTerrainMaterial && TerrainPaintCapture->TextureTarget)
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(this, TerrainPaintCapture->TextureTarget);

		UTexture* TerrainMaskTexture = Cast<UTexture>(TerrainPaintCapture->TextureTarget);

		if (TerrainMaskTexture)
		{
			FName MaskName = FName(TEXT("PlayerPositionMask"));
			FName MaxDistanceName = FName(TEXT("MaxDistance"));

			DynamicTerrainMaterial->SetTextureParameterValue(MaskName, TerrainMaskTexture);

			DynamicTerrainMaterial->SetScalarParameterValue(MaxDistanceName, OcclusionMaxDistance);
		}
	}

	if (DynamicAccentMaterial)
	{
		FName MaxDistanceName = FName(TEXT("MaxDistance"));

		DynamicTerrainMaterial->SetScalarParameterValue(MaxDistanceName, OcclusionMaxDistance);
	}

	if (AreMeshMaterialsValid())
	{
		TerrainMeshComp->SetMaterial(0, DynamicTerrainMaterial);
		TerrainUnwrapMesh->SetMaterial(0, DynamicTerrainUnwrapMaterial);
		AccentMeshComp->SetMaterial(0, DynamicAccentMaterial);
	}
}

void AMapArea::SetDynamicAccentMeshMaterial(UMaterialInstanceDynamic* InDynamicMaterial)
{
	if (InDynamicMaterial)
	{
		DynamicAccentMaterial = InDynamicMaterial;
	}
}

void AMapArea::SetDynamicTerrainUnwrapMaterial(UMaterialInstanceDynamic* InDynamicMaterial)
{
	if (InDynamicMaterial)
	{
		DynamicTerrainUnwrapMaterial = InDynamicMaterial;
	}
}

void AMapArea::SetDynamicTerrainMeshMaterial(UMaterialInstanceDynamic* InDynamicMaterial)
{
	if (InDynamicMaterial)
	{
		DynamicTerrainMaterial = InDynamicMaterial;
	}
}

void AMapArea::SetTerrainPaintTextureTarget(UTextureRenderTarget2D* InNewTextureRenderTarget)
{
	if (InNewTextureRenderTarget)
	{
		TerrainPaintCapture->TextureTarget = InNewTextureRenderTarget;
	}
}

UStaticMeshComponent* AMapArea::GetTerrainMeshComponent()
{
	return TerrainMeshComp;
}

UStaticMeshComponent* AMapArea::GetAccentMeshComponent()
{
	return AccentMeshComp;
}

void AMapArea::GetZoneTag(FGameplayTag& OutGameplayTag)
{
	OutGameplayTag = ZoneTag;
}

void AMapArea::DiscoverMapArea()
{
	bHasBeenDiscovered = true;
}

bool AMapArea::IsMapAreaDiscovered()
{
	return bHasBeenDiscovered;
}

void AMapArea::CalculateMapEnvironmentTerrainPosition(FVector& OutTerrainWorldPosition)
{
	TArray<AActor*> TerrainMapActors{};
	TArray<AActor*> AccentMapActors{};

	GetTaggedMapElements(TerrainMapActors, AccentMapActors);

	// Calculate average tranform (position wise) of each terrain map element
	if (TerrainMapActors.Num() == 1)
	{
		OutTerrainWorldPosition = TerrainMapActors[0]->GetTransform().GetLocation();
	}
	else
	{
		// multiple actors use the average of their origins, with Z being the min of all origins. Rotation is identity for simplicity
		FVector Location(FVector::ZeroVector);

		double MinZ = DBL_MAX;

		for (AActor* Actor : TerrainMapActors)
		{
			FTransform ActorTransform = Actor->GetTransform();
			Location += ActorTransform.GetLocation();
			MinZ = FMath::Min(ActorTransform.GetLocation().Z, MinZ);
		}
		Location /= (float)TerrainMapActors.Num();
		Location.Z = MinZ;

		OutTerrainWorldPosition = Location;
	}
}

bool AMapArea::IsUsingHighResolutionRT()
{
	return bIsUsingHighResolutionRT;
}

//--------------------------------------------------------------------------------------------

// Protected Functions --------------------------------------------------------------------------------------------

void AMapArea::BeginPlay()
{
	Super::BeginPlay();
}

void AMapArea::ToggleMeshSceneCaptureVisibility()
{
	SetMeshComponentSceneCaptureVisibility(!bIsMeshOnlyVisibleInSceneCapture);
}

void AMapArea::AlignAccentMesh()
{
	TArray<AActor*> TerrainMapActors{};
	TArray<AActor*> AccentMapActors{};
	GetTaggedMapElements(TerrainMapActors, AccentMapActors);

	if (TerrainMapActors.IsEmpty() || AccentMapActors.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("MapArea: Cannot align accent mesh - missing tagged actors for zone %s"), *ZoneTag.ToString());
		return;
	}

	auto ComputeGroupOrigin = [](const TArray<AActor*>& Actors) -> FVector
		{
			if (Actors.Num() == 1)
			{
				return Actors[0]->GetActorLocation();
			}

			FVector AccumulatedLocation = FVector::ZeroVector;
			double MinZ = DBL_MAX;

			for (const AActor* Actor : Actors)
			{
				FVector Loc = Actor->GetActorLocation();
				AccumulatedLocation += Loc;
				MinZ = FMath::Min(Loc.Z, MinZ);
			}

			AccumulatedLocation /= static_cast<double>(Actors.Num());
			AccumulatedLocation.Z = MinZ;
			return AccumulatedLocation;
		};

	FVector TerrainGroupOrigin = ComputeGroupOrigin(TerrainMapActors);
	FVector AccentGroupOrigin = ComputeGroupOrigin(AccentMapActors);

	// Game-world offset between the two groups
	FVector GameWorldOffset = AccentGroupOrigin - TerrainGroupOrigin;

	FVector TerrainMeshRelativeOffset = TerrainMeshComp ? TerrainMeshComp->GetRelativeLocation() : FVector::ZeroVector;
	FVector FinalAccentRelativePosition = TerrainMeshRelativeOffset + GameWorldOffset;

	SetAccentRelativePosition(FinalAccentRelativePosition);

	UE_LOG(LogTemp, Display,
		TEXT("MapArea: Aligned accent mesh. TerrainOrigin=%s AccentOrigin=%s GameWorldDelta=%s TerrainMeshOffset=%s Final=%s"),
		*TerrainGroupOrigin.ToString(),
		*AccentGroupOrigin.ToString(),
		*GameWorldOffset.ToString(),
		*TerrainMeshRelativeOffset.ToString(),
		*FinalAccentRelativePosition.ToString());
}

void AMapArea::AlignMapAreas()
{
	TArray<AActor*> MapAreas{};
	TArray<FVector> MapAreaTerrainPositions{};

	if (GEngine)
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMapArea::StaticClass(), MapAreas);

		// Average position for map environment actors with terrain tag from the starting zone
		FVector StartingZoneTerrainAverageWorldPosition;

		// Map area with starting zone tag
		FVector StartingMapAreaPosition;

		for (AActor* Actor : MapAreas)
		{
			AMapArea* MapArea = Cast<AMapArea>(Actor);

			// Checking to see if map area is starting zone map area
			if (MapArea)
			{
				FGameplayTag StartingMapAreaZoneTag = FGameplayTag::RequestGameplayTag(FName(TEXT("AutoMapGameplay.MapZone_1")));

				FGameplayTag MapAreaZoneTag;
				MapArea->GetZoneTag(MapAreaZoneTag);

				FVector MapAreaMapTerrainWorldPosition;

				// If it is calculate average position for terrain map environment actors and get actor location
				if (MapAreaZoneTag.MatchesTagExact(StartingMapAreaZoneTag))
				{
					StartingMapAreaPosition = MapArea->GetActorLocation();
					MapArea->CalculateMapEnvironmentTerrainPosition(MapAreaMapTerrainWorldPosition);
					StartingZoneTerrainAverageWorldPosition = MapAreaMapTerrainWorldPosition;
				}
				else
				{
					MapArea->CalculateMapEnvironmentTerrainPosition(MapAreaMapTerrainWorldPosition);
				}

				MapAreaTerrainPositions.Add(MapAreaMapTerrainWorldPosition);
			}
		}

		int32 MapAreaIndex{ 0 };
		for (const FVector& TerrainWorldPositions : MapAreaTerrainPositions)
		{
			// Calculating relative position of the terrain position from the starting zones positition
			FVector MapAreaRelativePosition = TerrainWorldPositions - StartingZoneTerrainAverageWorldPosition;
			FVector MapAreaWorldPosition = StartingMapAreaPosition + MapAreaRelativePosition;

			// Setting map area positon 
			if (MapAreas[MapAreaIndex])
			{
				MapAreas[MapAreaIndex]->SetActorLocation(MapAreaWorldPosition);
			}

			MapAreaIndex++;
		}
	}
}

void AMapArea::BakeGameWorldOrigin()
{
	TArray<AActor*> TerrainActors{};
	TArray<AActor*> AccentActors{};

	GetTaggedMapElements(TerrainActors, AccentActors);

	if (TerrainActors.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("MapArea: No terrain actors found for zone tag, cannot bake origin."));
		return;
	}

	if (TerrainActors.Num() == 1)
	{
		GameWorldOrigin = TerrainActors[0]->GetActorLocation();
	}
	else
	{
		FVector AccumulatedLocation = FVector::ZeroVector;
		double MinZ = DBL_MAX;

		for (AActor* Actor : TerrainActors)
		{
			FVector Loc = Actor->GetActorLocation();
			AccumulatedLocation += Loc;
			MinZ = FMath::Min(Loc.Z, MinZ);
		}

		AccumulatedLocation /= static_cast<double>(TerrainActors.Num());
		AccumulatedLocation.Z = MinZ;

		GameWorldOrigin = AccumulatedLocation;
	}

	UE_LOG(LogTemp, Display, TEXT("MapArea: Baked GameWorldOrigin to %s"), *GameWorldOrigin.ToString());
}

void AMapArea::SetMeshComponentSceneCaptureVisibility(bool InNewState)
{
	if (TerrainMeshComp && AccentMeshComp)
	{
		if (TerrainMeshComp->GetStaticMesh())
		{
			bIsMeshOnlyVisibleInSceneCapture = InNewState;
			TerrainMeshComp->SetVisibleInSceneCaptureOnly(InNewState);
			AccentMeshComp->SetVisibleInSceneCaptureOnly(InNewState);
		}
	}
}

void AMapArea::SetAccentRelativePosition(const FVector& InVector)
{
	AccentSceneComponent->SetRelativeLocation(InVector);
}

void AMapArea::CalculateViewParameters(float& OutOrthoWidth, FVector& OutRelativeLocation, FVector& OutUnrwapLocation)
{
	// If we have a valid terrain mesh
	if (TerrainMeshComp->GetStaticMesh())
	{
		FVector MinLocal;
		FVector MaxLocal;

		float DivConstant{ 5.0f };

		FTransform ActorTransform = GetActorTransform();

		TerrainMeshComp->GetLocalBounds(MinLocal, MaxLocal);

		// Getting orthowidth of capture component
		float OrthoWidth = FMath::Max(MaxLocal.X, MaxLocal.Y);
		OrthoWidth = FMath::Max(static_cast<double>(OrthoWidth), MaxLocal.Z);
		OutOrthoWidth = OrthoWidth + (OrthoWidth / DivConstant);

		// Getting world space coordinates of local bounds
		FVector TransformedMin = ActorTransform.TransformPosition(MinLocal);
		FVector TransformedMax = ActorTransform.TransformPosition(MaxLocal);

		FVector TransformedMid = (TransformedMin + TransformedMax) / 2.0f;
		FVector MidTransRelative = TransformedMid - GetActorLocation();

		// Getting position of scene capture component
		OutRelativeLocation.X = MidTransRelative.X;
		OutRelativeLocation.Y = MidTransRelative.Y;
		OutRelativeLocation.Z = MaxLocal.Z + (OrthoWidth / DivConstant);

		// Getting unwrap position of material
		OutUnrwapLocation.X = TransformedMid.X;
		OutUnrwapLocation.Y = TransformedMid.Y;
		OutUnrwapLocation.Z = TransformedMin.Z;
	}
}

void AMapArea::SetupCaptureParameters()
{
	float OrthoWidth;
	FVector SceneCaptureRelativeLoc;

	CalculateViewParameters(OrthoWidth, SceneCaptureRelativeLoc, UnwrapLocation);

	// Setting up capture component parameters
	TerrainPaintCapture->SetRelativeLocation(SceneCaptureRelativeLoc);
	TerrainPaintCapture->OrthoWidth = OrthoWidth;

	TerrainPaintCapture->ShowOnlyComponents.Add(TerrainUnwrapMesh);
}

void AMapArea::SetupTerrainUnwrapMesh()
{
	if (TerrainMeshComp->GetStaticMesh())
	{
		TerrainUnwrapMesh->SetStaticMesh(TerrainMeshComp->GetStaticMesh());
	}
}

void AMapArea::HandlePaintBehaviour(float DeltaTime)
{
	// Only paint when the map is closed and we are the active map area
	if (!bIsMapOpen && IsActiveMapArea())
	{
		FVector MarkerPosition;

		PaintTimer += DeltaTime;

		if (PaintTimer >= PaintRate)
		{
			PaintTimer = 0.0f;

			if (GetPlayerMarkerPosition(MarkerPosition))
			{
				PaintArea(MarkerPosition, TerrainPaintRadius);
			}
		}
	}
}

void AMapArea::HandleViewOcclusionBehaviour()
{
	if (AreMeshMaterialsValid())
	{
		FVector CameraPosition;

		if (GetMapViewerPosition(CameraPosition))
		{
			FName CameraPositionName = FName(TEXT("CameraPosition"));

			DynamicTerrainMaterial->SetVectorParameterValue(CameraPositionName, CameraPosition);
			DynamicAccentMaterial->SetVectorParameterValue(CameraPositionName, CameraPosition);
		}
	}
}

bool AMapArea::AreUnwrapMaterialsValid()
{
	if (DynamicTerrainUnwrapMaterial)
	{
		return true;
	}

	return false;
}

bool AMapArea::AreMeshMaterialsValid()
{
	return DynamicTerrainMaterial && DynamicAccentMaterial;
}

bool AMapArea::GetPlayerMarkerPosition(FVector& OutVector)
{
	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance)
	{
		UMap3DGameInstanceSubsystem* Map3DGISubSystem = GameInstance->GetSubsystem<UMap3DGameInstanceSubsystem>();

		if (Map3DGISubSystem)
		{
			Map3DGISubSystem->GetPlayerMarkerPosition(OutVector);
			return true;
		}
	}
	return false;
}

bool AMapArea::GetMapViewerPosition(FVector& OutVector)
{
	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance)
	{
		UMap3DGameInstanceSubsystem* Map3DGISubSystem = GameInstance->GetSubsystem<UMap3DGameInstanceSubsystem>();

		if (Map3DGISubSystem)
		{
			Map3DGISubSystem->GetMapViewerPosition(OutVector);
			return true;
		}
	}
	return false;
}

bool AMapArea::IsActiveMapArea()
{
	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance)
	{
		UMap3DGameInstanceSubsystem* Map3DGISubSystem = GameInstance->GetSubsystem<UMap3DGameInstanceSubsystem>();

		if (Map3DGISubSystem)
		{
			return Map3DGISubSystem->IsMapAreaActive(this);
		}
	}
	return false;
}

void AMapArea::GetTaggedMapElements(TArray<AActor*>& TerrainActors, TArray<AActor*>& AccentActors)
{
	TArray<AActor*> AutoMapActors{};

	FString ZoneTagStringName = ZoneTag.ToString();

	FString ZoneTagTerrainStringName = ZoneTag.ToString() + FString(TEXT(".Terrain"));
	FString ZoneTagAccentStringName = ZoneTag.ToString() + FString(TEXT(".Accent"));

	FGameplayTag TerrainGameplayTag = FGameplayTag::RequestGameplayTag(*ZoneTagTerrainStringName);
	FGameplayTag AccentGameplayTag = FGameplayTag::RequestGameplayTag(*ZoneTagAccentStringName);

	if (GEngine && TerrainGameplayTag.IsValid() && AccentGameplayTag.IsValid())
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAutoMapSMActor::StaticClass(), AutoMapActors);

		for (AActor* WorldActor : AutoMapActors)
		{
			if (WorldActor)
			{
				AAutoMapSMActor* StaticMeshMapActor = Cast<AAutoMapSMActor>(WorldActor);

				if (StaticMeshMapActor)
				{
					FGameplayTagContainer EnvironmentActorTagContainer{};

					const FGameplayTag& ZoningTag = StaticMeshMapActor->GetMapZoneTag();

					if (ZoningTag.MatchesTagExact(TerrainGameplayTag))
					{
						TerrainActors.AddUnique(WorldActor);
					}
					else if (ZoningTag.MatchesTagExact(AccentGameplayTag))
					{
						AccentActors.AddUnique(WorldActor);
					}
				}
			}
		}

		AutoMapActors.Empty();

		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAutoMapHISMActor::StaticClass(), AutoMapActors);

		for (AActor* WorldActor : AutoMapActors)
		{
			if (WorldActor)
			{
				AAutoMapHISMActor* HISMAutoMapActor = Cast<AAutoMapHISMActor>(WorldActor);

				if (HISMAutoMapActor)
				{
					FGameplayTagContainer EnvironmentActorTagContainer{};

					const FGameplayTag& ZoningTag = HISMAutoMapActor->GetMapZoneTag();

					if (ZoningTag.MatchesTagExact(TerrainGameplayTag))
					{
						TerrainActors.AddUnique(WorldActor);
					}
					else if (ZoningTag.MatchesTagExact(AccentGameplayTag))
					{
						AccentActors.AddUnique(WorldActor);
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------