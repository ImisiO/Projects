#include "Visualizers/AutoMapMeshTagVisualizer.h"

#include "AutoMapTagComponent.h"
#include "AutoMapSMActor.h"
#include "AutoMapHISMActor.h"

#include "SceneManagement.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Engine/Canvas.h"

#include "Editor.h"
#include "Engine/Selection.h"

// Public Functions --------------------------------------------------------------------------------------------

void FAutoMapMeshTagVisualizer::DrawVisualization(const UActorComponent* pComponent, const FSceneView* pView, FPrimitiveDrawInterface* pPDI)
{
    const UAutoMapTagComponent* TagComp = Cast<UAutoMapTagComponent>(pComponent);
    if (!TagComp) 
    {
        return;
    }

    AActor* pOwnerActor = TagComp->GetOwner();
    if (!pOwnerActor) 
    {
        return;
    }

    UWorld* pWorld = pOwnerActor->GetWorld();
    if (!pWorld) 
    {
        return;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(pWorld, AAutoMapSMActor::StaticClass(), AllActors);

    TArray<AActor*> HISMActors;
    UGameplayStatics::GetAllActorsOfClass(pWorld, AAutoMapHISMActor::StaticClass(), HISMActors);
    AllActors.Append(HISMActors);

    FVector OwnerOrigin;
    FVector OwnerExtent;
    GetMeshComponentBounds(pOwnerActor, OwnerOrigin, OwnerExtent);

    FMatrix OwnerBoundsMatrix = FRotationMatrix(pOwnerActor->GetActorRotation());
    OwnerBoundsMatrix.SetOrigin(OwnerOrigin);

    const FLinearColor CombineColour = TagComp->MapMeshCombineTag.IsValid()
        ? GetColourForTag(TagComp->MapMeshCombineTag.ToString())
        : FLinearColor::Gray;

    const FLinearColor ZoneColour = TagComp->MapZoneTag.IsValid()
        ? GetColourForTag(TagComp->MapZoneTag.ToString())
        : FLinearColor::Gray;

    DrawWireDiamond(pPDI, OwnerBoundsMatrix, 12.f, CombineColour, SDPG_Foreground, 1.5f);
    DrawWireDiamond(pPDI, OwnerBoundsMatrix, 20.f, ZoneColour, SDPG_Foreground, 1.5f);

    for (AActor* pOtherActor : AllActors)
    {
        if (!pOtherActor || pOtherActor == pOwnerActor) 
        {
            continue;
        }

        UAutoMapTagComponent* OtherTagComp = pOtherActor->FindComponentByClass<UAutoMapTagComponent>();
        if (!OtherTagComp) 
        {
            continue;
        }

        FVector OtherOrigin;
        FVector OtherExtent;
        GetMeshComponentBounds(pOtherActor, OtherOrigin, OtherExtent);

        FMatrix OtherBoundsMatrix = FRotationMatrix(pOtherActor->GetActorRotation());
        OtherBoundsMatrix.SetOrigin(OtherOrigin);

        // Offset so the two lines run parallel rather than overlapping
        const float OffsetAmount = 8.f;
        const FVector LineOffset = GetLineOffset(OwnerOrigin, OtherOrigin, pView, OffsetAmount);

        const FVector CombineStart = OwnerOrigin - LineOffset;
        const FVector CombineEnd = OtherOrigin - LineOffset;
        const FVector ZoneStart = OwnerOrigin + LineOffset;
        const FVector ZoneEnd = OtherOrigin + LineOffset;

        const bool bCombineMatch = TagComp->MapMeshCombineTag.IsValid() && OtherTagComp->MapMeshCombineTag == TagComp->MapMeshCombineTag;

        if (bCombineMatch)
        {
            pPDI->DrawLine(CombineStart, CombineEnd, CombineColour, SDPG_Foreground, 1.5f);
            DrawWireDiamond(pPDI, OtherBoundsMatrix, 12.f, CombineColour, SDPG_Foreground, 1.5f);
        }
        else
        {
            const FLinearColor GreyedCombine = FLinearColor(CombineColour.R * 0.25f, CombineColour.G * 0.25f, CombineColour.B * 0.25f, 0.4f);

            pPDI->DrawLine(CombineStart, CombineEnd, GreyedCombine, SDPG_World, 0.5f);
            DrawWireDiamond(pPDI, OtherBoundsMatrix, 12.f, GreyedCombine, SDPG_World, 0.5f);
        }

        const bool bZoneMatch = TagComp->MapZoneTag.IsValid() && OtherTagComp->MapZoneTag == TagComp->MapZoneTag;

        if (bZoneMatch)
        {
            pPDI->DrawLine(ZoneStart, ZoneEnd, ZoneColour, SDPG_Foreground, 1.5f);
            DrawWireDiamond(pPDI, OtherBoundsMatrix, 20.f, ZoneColour, SDPG_Foreground, 1.5f);
        }
        else
        {
            const FLinearColor GreyedZone = FLinearColor(ZoneColour.R * 0.25f, ZoneColour.G * 0.25f, ZoneColour.B * 0.25f, 0.4f);

            pPDI->DrawLine(ZoneStart, ZoneEnd, GreyedZone, SDPG_World, 0.5f);
            DrawWireDiamond(pPDI, OtherBoundsMatrix, 20.f, GreyedZone, SDPG_World, 0.5f);
        }
    }
}

void FAutoMapMeshTagVisualizer::DrawVisualizationHUD(const UActorComponent* pComponent, const FViewport* pViewport, const FSceneView* pView, FCanvas* pCanvas)
{
    const UAutoMapTagComponent* TagComp = Cast<UAutoMapTagComponent>(pComponent);
    if (!TagComp) 
    {
        return;
    }

    AActor* OwnerActor = TagComp->GetOwner();
    if (!OwnerActor)
    { 
        return;
    }

    UWorld* World = OwnerActor->GetWorld();
    if (!World) 
    {
        return;
    }

    USelection* Selection = GEditor->GetSelectedActors();
    AActor* FirstAutoMapSelected = nullptr;

    if (Selection)
    {
        for (int32 i = 0; i < Selection->Num(); i++)
        {
            AActor* SelectedActor = Cast<AActor>(Selection->GetSelectedObject(i));
            if (SelectedActor && SelectedActor->FindComponentByClass<UAutoMapTagComponent>())
            {
                FirstAutoMapSelected = SelectedActor;
                break;
            }
        }
    }

    if (!FirstAutoMapSelected || FirstAutoMapSelected != OwnerActor) 
    {
        return;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AAutoMapSMActor::StaticClass(), AllActors);

    TArray<AActor*> HISMActors;
    UGameplayStatics::GetAllActorsOfClass(World, AAutoMapHISMActor::StaticClass(), HISMActors);
    AllActors.Append(HISMActors);

    for (AActor* pActor : AllActors)
    {
        if (!pActor)
        {
            continue;
        }

        UAutoMapTagComponent* pActorTagComp = pActor->FindComponentByClass<UAutoMapTagComponent>();
        if (!pActorTagComp) 
        {
            continue;
        }

        FVector ActorOrigin;
        FVector ActorExtent;
        GetMeshComponentBounds(pActor, ActorOrigin, ActorExtent);

        FVector2D ScreenPos;
        FVector TextWorldPos = ActorOrigin + FVector(0.f, 0.f, ActorExtent.Z + 20.f);

        if (!pView->WorldToPixel(TextWorldPos, ScreenPos))
        { 
            continue;
        }

        // Slightly brighter text for selected actors
        const bool bIsSelected = pActor->IsSelected();
        
        const FLinearColor LabelColour = bIsSelected
            ? FLinearColor(1.0f, 1.0f, 1.0f)
            : FLinearColor(0.65f, 0.65f, 0.65f);
        
        const FVector2D TextScale = bIsSelected
            ? FVector2D(1.2f, 1.2f)
            : FVector2D(1.0f, 1.0f);

        // Actor label
        FCanvasTextItem LabelItem(ScreenPos, FText::FromString(pActor->GetActorLabel()), GEngine->GetSmallFont(), LabelColour);
        LabelItem.bOutlined = true;
        LabelItem.OutlineColor = FLinearColor::Black;
        LabelItem.Scale = TextScale;
        pCanvas->DrawItem(LabelItem);

        FString CombineText = pActorTagComp->MapMeshCombineTag.IsValid()
            ? FString(TEXT("[MC] ")) + pActorTagComp->MapMeshCombineTag.ToString()
            : TEXT("[MC] No Tag");

        FCanvasTextItem CombineItem(FVector2D(ScreenPos.X, ScreenPos.Y + 14.f), FText::FromString(CombineText), GEngine->GetSmallFont(),
            pActorTagComp->MapMeshCombineTag.IsValid()
            ? GetColourForTag(pActorTagComp->MapMeshCombineTag.ToString())
            : FLinearColor::Gray);

        CombineItem.bOutlined = true;
        CombineItem.OutlineColor = FLinearColor::Black;
        CombineItem.Scale = TextScale;
        pCanvas->DrawItem(CombineItem);

        FString ZoneText = pActorTagComp->MapZoneTag.IsValid()
            ? FString(TEXT("[GZ] ")) + pActorTagComp->MapZoneTag.ToString()
            : TEXT("[GZ] No Tag");

        FCanvasTextItem ZoneItem(FVector2D(ScreenPos.X, ScreenPos.Y + 28.f), FText::FromString(ZoneText), GEngine->GetSmallFont(),
            pActorTagComp->MapZoneTag.IsValid()
            ? GetColourForTag(pActorTagComp->MapZoneTag.ToString())
            : FLinearColor::Gray);

        ZoneItem.bOutlined = true;
        ZoneItem.OutlineColor = FLinearColor::Black;
        ZoneItem.Scale = TextScale;
        pCanvas->DrawItem(ZoneItem);
    }
}

//--------------------------------------------------------------------------------------------

// Protected Functions --------------------------------------------------------------------------------------------

FLinearColor FAutoMapMeshTagVisualizer::GetColourForTag(const FString& TagString) const
{
    if (const FLinearColor* pCached = TagColourCache.Find(TagString))
    {
        return *pCached;
    }

    uint32 Hash = GetTypeHash(TagString);
    float Hue = (Hash % 360) / 360.f;

    FLinearColor NewColour = FLinearColor::MakeFromHSV8(uint8(Hue * 255.f), 200, 220);

    TagColourCache.Add(TagString, NewColour);
    return NewColour;
}

void FAutoMapMeshTagVisualizer::GetMeshComponentBounds(AActor* pActor, FVector& OutOrigin, FVector& OutExtent)
{
    UHierarchicalInstancedStaticMeshComponent* pHISMComp = pActor->FindComponentByClass<UHierarchicalInstancedStaticMeshComponent>();

    if (pHISMComp)
    {
        FBoxSphereBounds Bounds = pHISMComp->CalcBounds(pHISMComp->GetComponentTransform());
        OutOrigin = Bounds.Origin;
        OutExtent = Bounds.BoxExtent;
        return;
    }

    UStaticMeshComponent* pSMComp = pActor->FindComponentByClass<UStaticMeshComponent>();

    if (pSMComp)
    {
        FBoxSphereBounds Bounds = pSMComp->CalcBounds(pSMComp->GetComponentTransform());
        OutOrigin = Bounds.Origin;
        OutExtent = Bounds.BoxExtent;
        return;
    }

    pActor->GetActorBounds(false, OutOrigin, OutExtent, false);
}

FVector FAutoMapMeshTagVisualizer::GetLineOffset(const FVector& Start, const FVector& End, const FSceneView* pView, float OffsetAmount)
{
	FVector LineDir = (End - Start).GetSafeNormal();

	FVector CameraRight = pView->ViewMatrices.GetViewMatrix().GetColumn(0);

	FVector Offset = CameraRight - (FVector::DotProduct(CameraRight, LineDir) * LineDir);
	Offset.Normalize();

	return Offset * OffsetAmount;
}

//--------------------------------------------------------------------------------------------