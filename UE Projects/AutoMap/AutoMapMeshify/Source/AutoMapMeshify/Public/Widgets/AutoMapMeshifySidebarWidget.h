#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Notifications/SProgressBar.h"

DECLARE_LOG_CATEGORY_EXTERN(AutoMapMeshify, Log, All);

class UMeshManagerSubsystem;
class AAutoMapSMActor;
class AAutoMapHISMActor;

struct FAutoMapActorListItem
{
    FString TagName;
    TWeakObjectPtr<AActor> Actor;
    bool bSelected = false;
};

class SAutoMapMeshifySidebarWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SAutoMapMeshifySidebarWidget) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual ~SAutoMapMeshifySidebarWidget();

private:
    FReply OnRefreshListClicked();
    FReply OnConvertSelectedActorsClicked();
    FReply OnExportTaggedActorsClicked();
    FReply OnImportHoudiniClicked();

    UMeshManagerSubsystem* GetMeshManagerSubsystem() const;

    void RefreshActorList();
    TSharedRef<ITableRow> GenerateActorRow(TSharedPtr<FAutoMapActorListItem> Item, const TSharedRef<STableViewBase>& OwnerTable);

    FSlateColor GetProgressBarColour() const;
    TOptional<float> GetProgressBarPercentage() const;

    void SetStatus(const FString& Message, float InProgress = 0.0f, FLinearColor InProgressBarColour = FLinearColor(0.0f, 0.5f, 1.0f));

    void OnLevelActorAdded(AActor* Actor);
    void OnLevelActorDeleted(AActor* Actor);

#if WITH_EDITOR
	void OnAutoMapStaticMeshMeshTagChanged(AAutoMapSMActor* Actor);
	void OnAutoMapHISMMeshTagChanged(AAutoMapHISMActor* Actor);
#endif

    TArray<TSharedPtr<FAutoMapActorListItem>> ListItems;
    TSharedPtr<SListView<TSharedPtr<FAutoMapActorListItem>>> ListView;

    TSharedPtr<STextBlock> StatusTextBlock;
    TSharedPtr<SProgressBar> ProgressBar;

    FDelegateHandle OnActorAddedHandle;
    FDelegateHandle OnActorDeletedHandle;

#if WITH_EDITOR
    FDelegateHandle OnAutoMapSMMeshTagChangedHandle;
    FDelegateHandle OnAutoMapHISMMeshTagChangedHandle;
#endif

    FLinearColor ProgressBarColour = FLinearColor(0.0f, 0.5f, 1.0f);
    TOptional<float> ProgressValue = TOptional<float>(0.0f);

    float TagColumnWidth = 0.5f;
    float ActorColumnWidth = 0.5f;
};