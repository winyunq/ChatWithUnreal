// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#include "SChatInput.h"
#include "SAttachmentList.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SBoxPanel.h"
#include "ChatWithUnrealStyle.h"
#include "UmgMcpDelegates.h"

#include "ImageUtils.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "Input/DragAndDrop.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include "Editor.h"

namespace
{
	FText GetLocText(const FString& English, const FString& Chinese)
	{
		const FString Culture = FInternationalization::Get().GetCurrentCulture()->GetName();
		return FText::FromString(Culture.StartsWith(TEXT("zh")) ? Chinese : English);
	}
}

TWeakPtr<SChatInput> SChatInput::Instance = nullptr;

void SChatInput::Construct(const FArguments& InArgs)
{
	Instance = SharedThis(this);
	bIsUpdatingText = false;
	LastText = TEXT("");
	ActiveAtAgent = TEXT("");

	OnSendShortcutTriggeredEvent = InArgs._OnSendShortcutTriggered;
	OnPasteShortcutTriggeredEvent = InArgs._OnPasteShortcutTriggered;
	OnFilesDroppedEvent = InArgs._OnFilesDropped;

	ChildSlot
	[
		SAssignNew(AgentMenuAnchor, SMenuAnchor)
		.Placement(MenuPlacement_AboveAnchor)
		.OnGetMenuContent(FOnGetContent::CreateSP(this, &SChatInput::OnGenerateAgentMenu))
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("InputTextBox.Background.Normal"))
			.Padding(FMargin(4.0f, 2.0f, 4.0f, 2.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
				[
					SNew(SBorder)
					.Visibility_Lambda([this]() { return ActiveAtAgent.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible; })
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
								.Image_Lambda([this]() -> const FSlateBrush* {
									const FSlateBrush* Brush = nullptr;
									if (FUmgMcpDelegates::OnGetAgentAvatar.IsBound())
									{
										Brush = FUmgMcpDelegates::OnGetAgentAvatar.Execute(ActiveAtAgent);
									}
									if (!Brush)
									{
										Brush = FChatWithUnrealStyle::Get().GetBrush("ChatWithUnreal.Agent.Agent");
									}
									return Brush;
								})
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
						[
							SNew(STextBlock)
							.Text_Lambda([this]() { return FText::FromString(TEXT("@") + ActiveAtAgent); })
							.Font(FAppStyle::Get().GetFontStyle("BoldFont"))
						]
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SAssignNew(InputTextBox, SMultiLineEditableTextBox)
					.AutoWrapText(true)
					.HintText(GetLocText(TEXT("Type a message... (Ctrl+Enter to Send)"), TEXT("输入消息... (Ctrl+Enter 发送)")))
					.OnKeyDownHandler(this, &SChatInput::OnInputKeyDown)
					.OnTextChanged(this, &SChatInput::HandleTextChanged)
				]
			]
		]
	];
}

FText SChatInput::GetText() const
{
	if (!InputTextBox.IsValid()) return FText::GetEmpty();

	FString RawText = InputTextBox->GetText().ToString();
	FString ResultText;
	int32 ImageCounter = 0;

	for (int32 i = 0; i < RawText.Len(); ++i)
	{
		if (RawText[i] == 0x25C6) // Technology Diamond Character ◆
		{
			ImageCounter++;
			ResultText += FString::Printf(TEXT("<WinyunqImageBegin>image%d<WinyunqImageEnd>"), ImageCounter);
		}
		else
		{
			ResultText.AppendChar(RawText[i]);
		}
	}
	return FText::FromString(ResultText);
}

void SChatInput::SetText(const FText& InText)
{
	if (InputTextBox.IsValid())
	{
		FString RawText = InText.ToString();
		int32 SearchPos = 0;
		while ((SearchPos = RawText.Find(TEXT("<WinyunqImageBegin>"), ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchPos)) != INDEX_NONE)
		{
			int32 EndTagIdx = RawText.Find(TEXT("<WinyunqImageEnd>"), ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchPos + 19);
			if (EndTagIdx != INDEX_NONE)
			{
				RawText.RemoveAt(SearchPos, (EndTagIdx + 17) - SearchPos);
				RawText.InsertAt(SearchPos, TEXT("\u25C6"));
				SearchPos += 1;
			}
			else
			{
				SearchPos += 19;
			}
		}
		
		bIsUpdatingText = true;
		InputTextBox->SetText(FText::FromString(RawText));
		LastText = RawText;
		bIsUpdatingText = false;
	}
}

void SChatInput::ClearText()
{
	if (InputTextBox.IsValid())
	{
		bIsUpdatingText = true;
		InputTextBox->SetText(FText::GetEmpty());
		LastText = TEXT("");
		bIsUpdatingText = false;
	}
}

FReply SChatInput::OnInputKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// 发送快捷键：Enter（当未按Shift时）或Ctrl+Enter
	bool bIsEnter = InKeyEvent.GetKey() == EKeys::Enter;
	bool bHasCtrl = InKeyEvent.IsControlDown();
	bool bHasShift = InKeyEvent.IsShiftDown();

	if (bIsEnter && (!bHasShift || bHasCtrl))
	{
		OnSendShortcutTriggeredEvent.ExecuteIfBound();
		return FReply::Handled();
	}

	// 粘贴图片快捷键：Ctrl+V
	if (bHasCtrl && InKeyEvent.GetKey() == EKeys::V)
	{
		if (OnPasteShortcutTriggeredEvent.IsBound())
		{
			FReply Reply = OnPasteShortcutTriggeredEvent.Execute();
			if (Reply.IsEventHandled())
			{
				return Reply;
			}
		}
	}

	// 退格键拦截：当输入框为空且存在@智能体时，删除该智能体标签
	if (InKeyEvent.GetKey() == EKeys::BackSpace && InputTextBox.IsValid() && InputTextBox->GetText().IsEmpty() && !ActiveAtAgent.IsEmpty())
	{
		ActiveAtAgent.Empty();
		FUmgMcpDelegates::OnAtAgentChanged.Broadcast(TEXT(""));
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SChatInput::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FExternalDragOperation> ExternalDrag = DragDropEvent.GetOperationAs<FExternalDragOperation>();
	if (ExternalDrag.IsValid() && ExternalDrag->HasFiles())
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SChatInput::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FExternalDragOperation> ExternalDrag = DragDropEvent.GetOperationAs<FExternalDragOperation>();
	if (ExternalDrag.IsValid() && ExternalDrag->HasFiles())
	{
		if (OnFilesDroppedEvent.IsBound())
		{
			OnFilesDroppedEvent.Execute(ExternalDrag->GetFiles());
			return FReply::Handled();
		}
	}
	return FReply::Unhandled();
}

void SChatInput::RemoveImageTag(int32 TargetIndex)
{
	if (!InputTextBox.IsValid()) return;
	
	FString CurrentText = InputTextBox->GetText().ToString();
	int32 FoundCount = -1;
	for (int32 i = 0; i < CurrentText.Len(); ++i)
	{
		if (CurrentText[i] == 0x25C6) // ◆
		{
			FoundCount++;
			if (FoundCount == TargetIndex)
			{
				CurrentText.RemoveAt(i, 1);
				break;
			}
		}
	}
	
	bIsUpdatingText = true;
	InputTextBox->SetText(FText::FromString(CurrentText));
	LastText = CurrentText;
	bIsUpdatingText = false;
}

void SChatInput::InsertImageTag(const FString& InImageId)
{
	if (InputTextBox.IsValid())
	{
		bIsUpdatingText = true;
		InputTextBox->InsertTextAtCursor(FText::FromString(TEXT("\u25C6")));
		LastText = InputTextBox->GetText().ToString();
		bIsUpdatingText = false;
	}
}

void SChatInput::SetAttachmentList(TSharedPtr<SAttachmentList> InList)
{
	AttachmentListWidget = InList;
}

void SChatInput::HandleTextChanged(const FText& NewText)
{
	if (bIsUpdatingText) return;

	FString NewTextStr = NewText.ToString();

	// @ 智能体检测逻辑
	int32 AtIndex = INDEX_NONE;
	if (NewTextStr.FindLastChar(TEXT('@'), AtIndex))
	{
		// 确保 @ 后面没有空格，且是当前正在输入的过滤词
		FString FilterText = NewTextStr.Mid(AtIndex + 1);
		if (!FilterText.Contains(TEXT(" ")))
		{
			FilterAgentSuggestions(FilterText);
			if (AgentSuggestions.Num() > 0 && AgentMenuAnchor.IsValid())
			{
				AgentMenuAnchor->SetIsOpen(true);
			}
			else if (AgentMenuAnchor.IsValid())
			{
				AgentMenuAnchor->SetIsOpen(false);
			}
		}
		else if (AgentMenuAnchor.IsValid())
		{
			AgentMenuAnchor->SetIsOpen(false);
		}
	}
	else if (AgentMenuAnchor.IsValid())
	{
		AgentMenuAnchor->SetIsOpen(false);
	}

	TSharedPtr<SAttachmentList> AttList = AttachmentListWidget.Pin();
	if (AttList.IsValid())
	{
		// 1. 分别统计旧文本和新文本中的 ◆ 数量及物理索引位置
		TArray<int32> OldTokenIndices;
		TArray<int32> NewTokenIndices;

		for (int32 i = 0; i < LastText.Len(); ++i)
		{
			if (LastText[i] == 0x25C6) // ◆
			{
				OldTokenIndices.Add(i);
			}
		}

		for (int32 i = 0; i < NewTextStr.Len(); ++i)
		{
			if (NewTextStr[i] == 0x25C6) // ◆
			{
				NewTokenIndices.Add(i);
			}
		}

		const TArray<FAttachmentItem>& Items = AttList->GetAttachmentItems();

		// 2. 如果占位符数量减少了，代表用户通过打字编辑（如 Backspace 等）物理删除了占位符！
		if (NewTokenIndices.Num() < OldTokenIndices.Num() && Items.Num() > 0)
		{
			if (NewTokenIndices.Num() == 0)
			{
				bIsUpdatingText = true;
				AttList->ClearAttachments();
				bIsUpdatingText = false;
			}
			else
			{
				// 用前缀上下文比对算法，精准找出是哪个占位符被干掉了
				int32 ErasedIndex = -1;
				for (int32 i = 0; i < NewTokenIndices.Num(); ++i)
				{
					FString NewPrefix = NewTextStr.Left(NewTokenIndices[i]).Replace(TEXT("◆"), TEXT(""));
					FString OldPrefix = LastText.Left(OldTokenIndices[i]).Replace(TEXT("◆"), TEXT(""));

					if (NewPrefix != OldPrefix)
					{
						ErasedIndex = i;
						break;
					}
				}

				if (ErasedIndex == -1)
				{
					ErasedIndex = OldTokenIndices.Num() - 1;
				}

				// 双向联动：反向将对应的图片从附录中剔除销毁，实现完美的闭环状态一致性
				if (ErasedIndex >= 0 && ErasedIndex < Items.Num())
				{
					bIsUpdatingText = true;
					AttList->RemoveAttachmentById(Items[ErasedIndex].ImageId);
					bIsUpdatingText = false;
				}
			}
		}
	}

	LastText = NewTextStr;
}

TSharedRef<SWidget> SChatInput::OnGenerateAgentMenu()
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
				.OnGenerateRow_Lambda([](TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable) {
					return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
						.Padding(FMargin(8.0f, 4.0f))
						[
							SNew(STextBlock)
							.Text(Item.IsValid() ? FText::FromString(*Item) : FText::GetEmpty())
						];
				})
				.OnSelectionChanged_Lambda([this](TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo) {
					OnAgentSelected(SelectedItem, SelectInfo);
				})
			]
		];
}

void SChatInput::FilterAgentSuggestions(const FString& FilterText)
{
	AgentSuggestions.Empty();
	
	TArray<FString> CandidateNames;
	if (FUmgMcpDelegates::OnGetAvailableAgents.IsBound())
	{
		CandidateNames = FUmgMcpDelegates::OnGetAvailableAgents.Execute();
	}
	
	if (CandidateNames.Num() == 0)
	{
		CandidateNames.Add(TEXT("Layout"));
		CandidateNames.Add(TEXT("Widget"));
		CandidateNames.Add(TEXT("Master"));
	}

	for (const FString& Name : CandidateNames)
	{
		if (FilterText.IsEmpty() || Name.Contains(FilterText, ESearchCase::IgnoreCase))
		{
			AgentSuggestions.Add(MakeShared<FString>(Name));
		}
	}
	
	if (AgentListView.IsValid())
	{
		AgentListView->RequestListRefresh();
	}
}

void SChatInput::OnAgentSelected(TSharedPtr<FString> SelectedAgent, ESelectInfo::Type SelectInfo)
{
	if (!SelectedAgent.IsValid())
	{
		return;
	}

	if (InputTextBox.IsValid() && AgentMenuAnchor.IsValid())
	{
		ActiveAtAgent = *SelectedAgent;
		AgentMenuAnchor->SetIsOpen(false);

		// 移除用户输入的 @ 符号及其后的部分
		FString CurrentText = InputTextBox->GetText().ToString();
		int32 AtIndex = INDEX_NONE;
		if (CurrentText.FindLastChar(TEXT('@'), AtIndex))
		{
			CurrentText.RemoveAt(AtIndex, CurrentText.Len() - AtIndex);
		}

		bIsUpdatingText = true;
		InputTextBox->SetText(FText::FromString(CurrentText));
		LastText = CurrentText;
		bIsUpdatingText = false;

		// 广播给后端切换 AtAgent
		FUmgMcpDelegates::OnAtAgentChanged.Broadcast(ActiveAtAgent);
		
		// 重新聚焦输入框以防失焦
		FSlateApplication::Get().SetKeyboardFocus(InputTextBox);
	}
}

#undef LOCTEXT_NAMESPACE