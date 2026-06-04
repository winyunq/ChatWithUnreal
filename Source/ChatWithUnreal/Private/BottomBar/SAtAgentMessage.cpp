#include "BottomBar/SAtAgentMessage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Styling/AppStyle.h"
#include "ChatWithUnrealStyle.h"

TWeakPtr<SAtAgentMessage> SAtAgentMessage::Instance = nullptr;

void SAtAgentMessage::Construct(const FArguments& InArgs)
{
	Instance = SharedThis(this);
	OnClearClickedEvent = InArgs._OnClearClicked;
	OnAgentSelectedEvent = InArgs._OnAgentSelected;
	OnGetAgentAvatarEvent = InArgs._OnGetAgentAvatar;
	
	// 初始状态默认为折叠
	SetVisibility(EVisibility::Collapsed);

	ChildSlot
	[
		SAssignNew(AgentMenuAnchor, SMenuAnchor)
		.Placement(MenuPlacement_AboveAnchor)
		.OnGetMenuContent(this, &SAtAgentMessage::OnGenerateAgentMenu)
		.OnMenuOpenChanged_Lambda([this](bool bIsOpen) {
			if (!bIsOpen)
			{
				SetVisibility(AgentName.IsEmpty() ? EVisibility::Collapsed : EVisibility::SelfHitTestInvisible);
			}
		})
		[
			SNew(SBox)
			.Visibility_Lambda([this]() { return AgentName.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible; })
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("Menu.Background"))
				.Padding(FMargin(6.0f, 2.0f, 6.0f, 2.0f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(16.0f)
						.HeightOverride(16.0f)
						[
							SNew(SImage)
							.Image_Lambda([this]() { return AgentBrush; })
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.Text_Lambda([this]() { return FText::FromString(TEXT("@") + AgentName); })
						.Font(FAppStyle::Get().GetFontStyle("BoldFont"))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
					[
						SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
						.ContentPadding(0.0f)
						.OnClicked(this, &SAtAgentMessage::OnClearBtnClicked)
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush("Menu.Background"))
							.BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f, 0.8f))
							.Padding(FMargin(5.0f, 3.0f, 5.0f, 4.0f))
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("×")))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
								.ColorAndOpacity(FLinearColor(0.75f, 0.75f, 0.75f, 1.0f))
							]
						]
					]
				]
			]
		]
	];
}

void SAtAgentMessage::UpdateAgent(const FString& InAgentName, const struct FSlateBrush* InBrush)
{
	if (!InAgentName.IsEmpty())
	{
		AgentName = InAgentName;
		AgentBrush = InBrush;
		
		if (!AgentBrush)
		{
			AgentBrush = FChatWithUnrealStyle::Get().GetBrush("ChatWithUnreal.Agent.Agent");
		}
	}
	else
	{
		AgentName.Empty();
		AgentBrush = nullptr;
	}

	SetVisibility(AgentName.IsEmpty() ? EVisibility::Collapsed : EVisibility::SelfHitTestInvisible);
}

void SAtAgentMessage::ShowSuggestionsMenu(const TArray<FString>& InSuggestions)
{
	AgentSuggestions.Empty();
	for (const FString& Name : InSuggestions)
	{
		AgentSuggestions.Add(MakeShared<FString>(Name));
	}

	if (AgentListView.IsValid())
	{
		AgentListView->RequestListRefresh();
	}

	if (AgentSuggestions.Num() > 0 && AgentMenuAnchor.IsValid())
	{
		SetVisibility(EVisibility::SelfHitTestInvisible);
		AgentMenuAnchor->SetIsOpen(true);
	}
	else if (AgentMenuAnchor.IsValid())
	{
		AgentMenuAnchor->SetIsOpen(false);
		SetVisibility(AgentName.IsEmpty() ? EVisibility::Collapsed : EVisibility::SelfHitTestInvisible);
	}
}

void SAtAgentMessage::CloseSuggestionsMenu()
{
	if (AgentMenuAnchor.IsValid())
	{
		AgentMenuAnchor->SetIsOpen(false);
	}
	SetVisibility(AgentName.IsEmpty() ? EVisibility::Collapsed : EVisibility::SelfHitTestInvisible);
}

FReply SAtAgentMessage::OnClearBtnClicked()
{
	OnClearClickedEvent.ExecuteIfBound();
	return FReply::Handled();
}

TSharedRef<SWidget> SAtAgentMessage::OnGenerateAgentMenu()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Menu.Background"))
		.Padding(FMargin(2.0f))
		[
			SNew(SBox)
			.WidthOverride(150.0f)
			.HeightOverride(120.0f)
			[
				SAssignNew(AgentListView, SListView<TSharedPtr<FString>>)
				.ItemHeight(24.0f)
				.ListItemsSource(&AgentSuggestions)
				.OnGenerateRow(this, &SAtAgentMessage::OnGenerateAgentRow)
				.OnSelectionChanged(this, &SAtAgentMessage::OnAgentSelected)
			]
		];
}

TSharedRef<ITableRow> SAtAgentMessage::OnGenerateAgentRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	// 动态去后端拉取头像，我们这里可以通过 OnGetAgentAvatarEvent 拿到对应 Avatar
	const FSlateBrush* Brush = nullptr;
	if (Item.IsValid())
	{
		if (OnGetAgentAvatarEvent.IsBound())
		{
			Brush = OnGetAgentAvatarEvent.Execute(*Item);
		}
	}
	if (!Brush)
	{
		Brush = FChatWithUnrealStyle::Get().GetBrush("ChatWithUnreal.Agent.Agent");
	}

	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		.Padding(FMargin(8.0f, 4.0f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(16.0f)
				.HeightOverride(16.0f)
				[
					SNew(SImage)
					.Image(Brush)
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(Item.IsValid() ? FText::FromString(*Item) : FText::GetEmpty())
			]
		];
}

void SAtAgentMessage::OnAgentSelected(TSharedPtr<FString> SelectedAgent, ESelectInfo::Type SelectInfo)
{
	if (!SelectedAgent.IsValid() || SelectInfo == ESelectInfo::OnNavigation)
	{
		return;
	}

	if (AgentMenuAnchor.IsValid())
	{
		AgentMenuAnchor->SetIsOpen(false);
	}

	if (OnAgentSelectedEvent.IsBound())
	{
		OnAgentSelectedEvent.Execute(*SelectedAgent);
	}
}
