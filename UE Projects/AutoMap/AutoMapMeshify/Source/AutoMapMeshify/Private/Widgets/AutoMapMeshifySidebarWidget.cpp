#include "Widgets/AutoMapMeshifySidebarWidget.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Styling/AppStyle.h"
#include "EditorSubsystem.h"
#include "Editor.h"

#include "Subsystems/MeshManagerSubsystem.h"

#include "AutoMapHISMActor.h"
#include "AutoMapSMActor.h"

DEFINE_LOG_CATEGORY(AutoMapMeshify);

#define LOCTEXT_NAMESPACE "AutoMapMeshifySidebarWidget"

// Colour Constants --------------------------------------------------------------------------------------------

namespace ProgressBarColoursConstants
{
    FLinearColor InProgressColour = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);
    FLinearColor ErrorColour = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f);
    FLinearColor FinishedColour = FLinearColor(0.1f, 0.7f, 0.2f, 1.0f); 
}

//--------------------------------------------------------------------------------------------

// UI Helpers --------------------------------------------------------------------------------------------

static TSharedRef<SWidget> MakeSectionLabel(const FText& Label)
{
    return SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("DetailsView.CategoryTop"))
        .Padding(FMargin(6.f, 4.f))
        [
            SNew(STextBlock)
                .Text(Label)
                .Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
                .Justification(ETextJustify::Center)
        ];
}

static TSharedRef<SWidget> MakeToolButton(
    const FText& Label,
    const FText& Tooltip,
    const FOnClicked& OnClicked)
{
    return SNew(SButton)
        .ContentPadding(FMargin(0.f, 6.f))
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        .ToolTipText(Tooltip)
        .OnClicked(OnClicked)
        [
            SNew(STextBlock)
                .Text(Label)
                .Justification(ETextJustify::Center)
        ];
}

//--------------------------------------------------------------------------------------------

// Public Functions --------------------------------------------------------------------------------------------

void SAutoMapMeshifySidebarWidget::Construct(const FArguments& InArgs)
{
    RefreshActorList();

    ChildSlot
        [
            SNew(SVerticalBox)

                // Actor List --------------------------------------------------
                + SVerticalBox::Slot()
                .FillHeight(1.f)
                [
                    SNew(SVerticalBox)

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            MakeSectionLabel(LOCTEXT("ActorListHeader", "AutoMap Mesh Actors"))
                        ]

                        + SVerticalBox::Slot()
                        .FillHeight(1.f)
                        [
                            SNew(SBorder)
                                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                                [
                                    SAssignNew(ListView, SListView<TSharedPtr<FAutoMapActorListItem>>)
                                        .ListItemsSource(&ListItems)
                                        .SelectionMode(ESelectionMode::Multi)
                                        .OnGenerateRow(this, &SAutoMapMeshifySidebarWidget::GenerateActorRow)
                                        .HeaderRow
                                        (
                                            SNew(SHeaderRow)

                                            + SHeaderRow::Column("Tag")
                                            .DefaultLabel(LOCTEXT("ColTag", "Tag"))
                                            .FillWidth(TAttribute<float>::CreateLambda(
                                                [this]() { return TagColumnWidth; }))
                                            .OnWidthChanged_Lambda(
                                                [this](float NewWidth) { TagColumnWidth = NewWidth; })
                                            
                                            + SHeaderRow::Column("Actor")
                                            .DefaultLabel(LOCTEXT("ColActor", "Actor"))
                                            .FillWidth(TAttribute<float>::CreateLambda(
                                                [this]() { return ActorColumnWidth; }))
                                            .OnWidthChanged_Lambda(
                                                [this](float NewWidth) { ActorColumnWidth = NewWidth; })
                                        )
                                ]
                        ]
                ]

            // Buttons + Status dock ---------------------------
            + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SBorder)
                        .BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
                        .Padding(FMargin(4.f, 6.f))
                        [
                            SNew(SVerticalBox)

                                // Refresh list button
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(2.f, 2.f)
                                [
                                    MakeToolButton(
                                        LOCTEXT("RefreshList", "Refresh Actor List"),
                                        LOCTEXT("RefreshListTip", "Re-query all AutoMap mesh actors in the level."),
                                        FOnClicked::CreateSP(this, &SAutoMapMeshifySidebarWidget::OnRefreshListClicked))
                                ]

                                // Convert button
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(2.f, 2.f)
                                [
                                    MakeToolButton(
                                        LOCTEXT("ConvertSelected", "Convert Selected Actors"),
                                        LOCTEXT("ConvertSelectedTip", "Convert selected UE actors ↔ AutoMap actors."),
                                        FOnClicked::CreateSP(this, &SAutoMapMeshifySidebarWidget::OnConvertSelectedActorsClicked))
                                ]

                                // Export button
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(2.f, 2.f)
                                [
                                    MakeToolButton(
                                        LOCTEXT("ConvertTaggedAMActors", "Export Tagged AM Actors"),
                                        LOCTEXT("ConvertTaggedAMActorsTip", "Exports all tagged auto map actors into separate meshes depending on their tag"),
                                        FOnClicked::CreateSP(this, &SAutoMapMeshifySidebarWidget::OnExportTaggedActorsClicked))
                                ]

                                // Import button
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(2.f, 2.f)
                                [
                                    MakeToolButton(
                                        LOCTEXT("ImportHoudini", "Imports Meshes Generated By Houdini"),
                                        LOCTEXT("ImportHoudiniTip", "Imports all .obj files found in the AutoMapMeshes/HoudiniImport/... as static meshes"),
                                        FOnClicked::CreateSP(this, &SAutoMapMeshifySidebarWidget::OnImportHoudiniClicked))
                                ]

                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0.f, 6.f, 0.f, 4.f)
                                [
                                    SNew(SSeparator)
                                ]

                                // Progress bar
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(2.f, 0.f, 2.f, 4.f)
                                [
                                    SAssignNew(ProgressBar, SProgressBar)
                                        .Percent(this, &SAutoMapMeshifySidebarWidget::GetProgressBarPercentage)
                                        .FillColorAndOpacity(this, &SAutoMapMeshifySidebarWidget::GetProgressBarColour)
                                ]

                                // Status text
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(2.f, 0.f)
                                [
                                    SAssignNew(StatusTextBlock, STextBlock)
                                        .Text(LOCTEXT("ReadyStatus", "Ready"))
                                        .Font(FAppStyle::GetFontStyle("TinyText"))
                                        .Justification(ETextJustify::Center)
                                        .ColorAndOpacity(this, &SAutoMapMeshifySidebarWidget::GetProgressBarColour)
                                        .AutoWrapText(true)
                                ]
                        ]
                ]
        ];

		OnActorAddedHandle = GEngine->OnLevelActorAdded().AddSP(this, &SAutoMapMeshifySidebarWidget::OnLevelActorAdded);
		OnActorDeletedHandle = GEngine->OnLevelActorDeleted().AddSP(this, &SAutoMapMeshifySidebarWidget::OnLevelActorDeleted);

#if WITH_EDITOR
        OnAutoMapSMMeshTagChangedHandle = AAutoMapSMActor::OnAutoMapMeshTagChanged.AddSP(this, &SAutoMapMeshifySidebarWidget::OnAutoMapStaticMeshMeshTagChanged);
        OnAutoMapHISMMeshTagChangedHandle = AAutoMapHISMActor::OnAutoMapMeshTagChanged.AddSP(this, &SAutoMapMeshifySidebarWidget::OnAutoMapHISMMeshTagChanged);
#endif // WITH_EDITOR

}

SAutoMapMeshifySidebarWidget::~SAutoMapMeshifySidebarWidget()
{
    if (GEngine)
    {
        GEngine->OnLevelActorAdded().Remove(OnActorAddedHandle);
        GEngine->OnLevelActorDeleted().Remove(OnActorDeletedHandle);
    }

#if WITH_EDITOR
    if (OnAutoMapSMMeshTagChangedHandle.IsValid())
    {
        AAutoMapSMActor::OnAutoMapMeshTagChanged.Remove(OnAutoMapSMMeshTagChangedHandle);
        AAutoMapHISMActor::OnAutoMapMeshTagChanged.Remove(OnAutoMapHISMMeshTagChangedHandle);
    }
#endif // WITH_EDITOR
}

//--------------------------------------------------------------------------------------------

// Private Functions --------------------------------------------------------------------------------------------

FReply SAutoMapMeshifySidebarWidget::OnRefreshListClicked()
{
    SetStatus(TEXT("Refreshing..."), 0.5f, ProgressBarColoursConstants::InProgressColour);
    RefreshActorList();
    SetStatus(FString::Printf(TEXT("Found %d actor(s)."), ListItems.Num()), 1.0f, ProgressBarColoursConstants::FinishedColour);
    return FReply::Handled();
}

FReply SAutoMapMeshifySidebarWidget::OnConvertSelectedActorsClicked()
{
    UMeshManagerSubsystem* AutoMapMeshManagerSubsystem = GetMeshManagerSubsystem();
    if (AutoMapMeshManagerSubsystem)
    {
        SetStatus(TEXT("Converting..."), 0.5f, ProgressBarColoursConstants::InProgressColour);
        AutoMapMeshManagerSubsystem->ConvertSelectedActors();
        RefreshActorList();
        SetStatus(TEXT("Conversion complete.") , 1.0, ProgressBarColoursConstants::FinishedColour);
    }
    else
    {
        SetStatus(TEXT("Error: subsystem not found."), 0.5f, ProgressBarColoursConstants::ErrorColour);
    }
    return FReply::Handled();
}

FReply SAutoMapMeshifySidebarWidget::OnExportTaggedActorsClicked()
{
    UMeshManagerSubsystem* AutoMapMeshManagerSubsystem = GetMeshManagerSubsystem();
    if (AutoMapMeshManagerSubsystem)
    {
        SetStatus(TEXT("Exporting Meshes..."), 0.5f, ProgressBarColoursConstants::InProgressColour);
        AutoMapMeshManagerSubsystem->ConvertTaggedMapMeshActors();
        SetStatus(TEXT("Exporting Complete"), 1.0, ProgressBarColoursConstants::FinishedColour);
    }
    else
    {
        SetStatus(TEXT("Error: subsystem not found."), 0.5f, ProgressBarColoursConstants::ErrorColour);
    }

    return FReply::Handled();
}

FReply SAutoMapMeshifySidebarWidget::OnImportHoudiniClicked()
{
    UMeshManagerSubsystem* AutoMapMeshManagerSubsystem = GetMeshManagerSubsystem();
    if (AutoMapMeshManagerSubsystem)
    {
        SetStatus(TEXT("Importing Houdini Meshes..."), 0.5f, ProgressBarColoursConstants::InProgressColour);
        AutoMapMeshManagerSubsystem->ImportHoudiniExportMeshes();
        SetStatus(TEXT("Import Complete"), 1.0, ProgressBarColoursConstants::FinishedColour);
    }
    else
    {
        SetStatus(TEXT("Error: subsystem not found."), 0.5f, ProgressBarColoursConstants::ErrorColour);
    }

    return FReply::Handled();
}

UMeshManagerSubsystem* SAutoMapMeshifySidebarWidget::GetMeshManagerSubsystem() const
{
    if (GEditor)
    {
        return GEditor->GetEditorSubsystem<UMeshManagerSubsystem>();
    }

    return nullptr;
}

void SAutoMapMeshifySidebarWidget::RefreshActorList()
{
    ListItems.Empty();

    UMeshManagerSubsystem* AutoMapMeshManagerSubsystem = GetMeshManagerSubsystem();
    if (!AutoMapMeshManagerSubsystem) 
    {
        return;
    }

    TMap<FString, TArray<AActor*>> ActorMap;
    AutoMapMeshManagerSubsystem->GetAllAutoMapMeshActors(ActorMap);

    for (auto& Pair : ActorMap)
    {
        for (AActor* Actor : Pair.Value)
        {
            if (Actor)
            {
                TSharedPtr<FAutoMapActorListItem> Item = MakeShared<FAutoMapActorListItem>();
                Item->TagName = Pair.Key;
                Item->Actor = Actor;
                ListItems.Add(Item);
            }
        }
    }

    if (ListView.IsValid())
    {
        ListView->RequestListRefresh();
    }
}

TSharedRef<ITableRow> SAutoMapMeshifySidebarWidget::GenerateActorRow( TSharedPtr<FAutoMapActorListItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
    FString ActorName = Item->Actor.IsValid()
        ? Item->Actor->GetActorLabel()
        : TEXT("(invalid)");

    return SNew(STableRow<TSharedPtr<FAutoMapActorListItem>>, OwnerTable)
        [
            SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .FillWidth(TAttribute<float>::CreateLambda(
                    [this]() { return TagColumnWidth; }))
                .Padding(4.f, 2.f)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(Item->TagName))
                        .Font(FAppStyle::GetFontStyle("TinyText"))
                ]

                + SHorizontalBox::Slot()
                .FillWidth(TAttribute<float>::CreateLambda(
                    [this]() { return ActorColumnWidth; }))
                .Padding(4.f, 2.f)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(ActorName))
                        .Font(FAppStyle::GetFontStyle("TinyText"))
                ]
        ];
}

FSlateColor SAutoMapMeshifySidebarWidget::GetProgressBarColour() const
{
    return FSlateColor(ProgressBarColour);
}

TOptional<float> SAutoMapMeshifySidebarWidget::GetProgressBarPercentage() const
{
    return ProgressValue;
}

void SAutoMapMeshifySidebarWidget::SetStatus(const FString& Message, float InProgress, FLinearColor InProgressBarColour)
{
    if (StatusTextBlock.IsValid())
    {
        StatusTextBlock->SetText(FText::FromString(Message));
    }

    ProgressValue = TOptional<float>(InProgress);
    ProgressBarColour = InProgressBarColour;
}

void SAutoMapMeshifySidebarWidget::OnLevelActorAdded(AActor* Actor)
{
    if (Actor && (Actor->IsA<AAutoMapSMActor>() || Actor->IsA<AAutoMapHISMActor>()))
    {
        SetStatus(FString::Printf(TEXT("Actor added: %s"), *Actor->GetActorLabel()));
        RefreshActorList();
    }
}

void SAutoMapMeshifySidebarWidget::OnLevelActorDeleted(AActor* Actor)
{
    if (Actor && (Actor->IsA<AAutoMapSMActor>() || Actor->IsA<AAutoMapHISMActor>()))
    {
        SetStatus(FString::Printf(TEXT("Actor removed: %s"), *Actor->GetActorLabel()));
        RefreshActorList();
    }
}

#if WITH_EDITOR

void SAutoMapMeshifySidebarWidget::OnAutoMapStaticMeshMeshTagChanged(AAutoMapSMActor* Actor)
{
	SetStatus(FString::Printf(TEXT("Tag updated on: %s"), *Actor->GetActorLabel()));
    RefreshActorList();
}

void SAutoMapMeshifySidebarWidget::OnAutoMapHISMMeshTagChanged(AAutoMapHISMActor* Actor)
{
    SetStatus(FString::Printf(TEXT("Tag updated on: %s"), *Actor->GetActorLabel()));
    RefreshActorList();
}

#endif // WITH_EDITOR

//--------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE