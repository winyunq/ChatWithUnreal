// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#include "SMessageInteractionHub.h"
#include "SChatWelcome.h"
#include "MessageInteractionHub/Messages/SAgentResponseGroup.h"
#include "MessageInteractionHub/Messages/SSystemNotificationWidget.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"

void SMessageInteractionHub::Construct(const FArguments& InArgs)
{
	OnSessionSelectedEvent = InArgs._OnSessionSelected;
	OnSessionDeletedEvent = InArgs._OnSessionDeleted;

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SAssignNew(ScrollBoxWidget, SScrollBox)
			.ScrollBarVisibility(EVisibility::Collapsed)
			+ SScrollBox::Slot()
			[
				SAssignNew(MessageList, SVerticalBox)
			]
		]
		+ SOverlay::Slot()
		[
			SAssignNew(WelcomeWidget, SChatWelcome)
			.Visibility(this, &SMessageInteractionHub::GetWelcomeVisibility)
			.OnSessionSelected_Lambda([this](const FString& SessionId) {
				OnSessionSelectedEvent.ExecuteIfBound(SessionId);
			})
			.OnSessionDeleted_Lambda([this](const FString& SessionId) {
				OnSessionDeletedEvent.ExecuteIfBound(SessionId);
			})
		]
	];
}

void SMessageInteractionHub::StartAgentResponse(const FString& AgentName)
{
	// 虚拟对话核心：由前端自己决定如何渲染 Agent 消息
	ActiveResponseGroup = SNew(SAgentResponseGroup).AgentName(AgentName);
	AddMessageWidget(ActiveResponseGroup.ToSharedRef());
}

void SMessageInteractionHub::AppendActiveText(const FString& Content)
{
	if (ActiveResponseGroup.IsValid())
	{
		ActiveResponseGroup->AppendToCurrentTextOutputBlock(Content);
	}
}

void SMessageInteractionHub::SetActiveStatus(const FText& Status, bool bShowSpinner, const FLinearColor& Color)
{
	if (ActiveResponseGroup.IsValid())
	{
		ActiveResponseGroup->SetAgentStatus(Status, bShowSpinner, Color);
	}
}

void SMessageInteractionHub::AddUserMessage(const FString& Content, const TArray<FString>& Base64Images)
{
	// 虚拟对话核心：由前端决定如何渲染用户消息（目前复用 AgentResponseGroup 但标识为 User）
	TSharedRef<SAgentResponseGroup> UserMsg = SNew(SAgentResponseGroup).AgentName(TEXT("User"));
	UserMsg->AppendToCurrentTextOutputBlock(Content);
	// 如果有图片，后续逻辑可在此扩展
	AddMessageWidget(UserMsg);
}

void SMessageInteractionHub::AddSystemNotification(const FString& Content)
{
	AddMessageWidget(SNew(SSystemNotificationWidget).MessageText(Content));
}

void SMessageInteractionHub::OnSessionResumed()
{
	ClearMessages();
}

void SMessageInteractionHub::ClearMessages()
{
	if (MessageList.IsValid())
	{
		MessageList->ClearChildren();
		ActiveResponseGroup.Reset();
		RefreshVisibility();
	}
}

void SMessageInteractionHub::RefreshVisibility()
{
	bool bHasVisibleMessage = false;
	if (MessageList.IsValid())
	{
		for (int32 i = 0; i < MessageList->GetChildren()->Num(); ++i)
		{
			if (MessageList->GetChildren()->GetChildAt(i)->GetVisibility().IsVisible())
			{
				bHasVisibleMessage = true;
				break;
			}
		}
	}
	
	if (ScrollBoxWidget.IsValid())
	{
		ScrollBoxWidget->SetVisibility(bHasVisibleMessage ? EVisibility::Visible : EVisibility::Collapsed);
	}
}

EVisibility SMessageInteractionHub::GetWelcomeVisibility() const
{
	return (MessageList.IsValid() && MessageList->GetChildren()->Num() > 0) ? EVisibility::Collapsed : EVisibility::Visible;
}

void SMessageInteractionHub::AddMessageWidget(TSharedRef<SWidget> InWidget)
{
	if (MessageList.IsValid())
	{
		MessageList->AddSlot().AutoHeight()[InWidget];
		RefreshVisibility();
		if (ScrollBoxWidget.IsValid()) ScrollBoxWidget->ScrollToEnd();
	}
}

void SMessageInteractionHub::MoveWidgetToBottom(TSharedRef<SWidget> InWidget)
{
	if (MessageList.IsValid())
	{
		MessageList->RemoveSlot(InWidget);
		MessageList->AddSlot().AutoHeight()[InWidget];
	}
}

void SMessageInteractionHub::RemoveMessageWidget(TSharedRef<SWidget> InWidget)
{
	if (MessageList.IsValid())
	{
		MessageList->RemoveSlot(InWidget);
		RefreshVisibility();
	}
}
