#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"

class FAutoMapMeshTagVisualizer : public FComponentVisualizer
{
public:
    virtual void DrawVisualization(const UActorComponent* pComponent, const FSceneView* pView, FPrimitiveDrawInterface* pPDI) override;
    
    virtual void DrawVisualizationHUD(const UActorComponent* pComponent, const FViewport* pViewport, const FSceneView* pView, FCanvas* pCanvas) override;

private:
    FLinearColor GetColourForTag(const FString& TagString) const;

    void GetMeshComponentBounds(AActor* pActor, FVector& OutOrigin, FVector& OutExtent);
    
    FVector GetLineOffset(const FVector& Start, const FVector& End, const FSceneView* pView, float OffsetAmount);

    mutable TMap<FString, FLinearColor> TagColourCache;
};