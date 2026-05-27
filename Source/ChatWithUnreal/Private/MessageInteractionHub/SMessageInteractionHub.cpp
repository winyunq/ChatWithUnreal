// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#include "SMessageInteractionHub.h"
#include "FabServer/ChatSystem/UmgMcpSessionManagerSubsystem.h"
#include "FabServer/ChatSystem/UmgMcpActiveMessageSubsystem.h"
#include "Messages/SUserMessageWidget.h"
#include "Messages/SAgentResponseGroup.h"
#include "Messages/SSystemNotificationWidget.h"
#include "Editor.h"
#include "Styling/AppStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"

#include "SChatWelcome.h"

void SMessageInteractionHub::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(WelcomeWidget, SChatWelcome)
			.OnSessionSelected_Lambda([this](const FString& SessionId) {
				// 欢迎页面选择了会话，它会自动触发 ResumeSession，
				// 而我们监听了 OnSessionResumed，所以这里不需要额外操作。
			})
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
			.Padding(FMargin(8.0f))
			[
				SAssignNew(ScrollBoxWidget, SScrollBox)
				.ScrollBarVisibility(EVisibility::Collapsed)
				+ SScrollBox::Slot()
				[
					SAssignNew(MessageList, SVerticalBox)
				]
			]
		]
	];

	if (GEditor)
	{
		// 1. 同步会话历史
		USessionManagerSubsystem* SessionSubsystem = GEditor->GetEditorSubsystem<USessionManagerSubsystem>();
		if (SessionSubsystem)
		{
			SessionSubsystem->OnSessionResumed.AddSP(this, &SMessageInteractionHub::OnSessionResumed);
			SessionSubsystem->OnMessageAppended.AddSP(this, &SMessageInteractionHub::OnMessageAppended);
			SessionSubsystem->OnSessionDeleted.AddSP(this, &SMessageInteractionHub::OnSessionDeleted);

			if (const FUmgMcpSessionData* ActiveSession = SessionSubsystem->GetActiveSession())
			{
				OnSessionResumed(*ActiveSession);
			}
		}

		// 2. 核心：将 Hub 注册给活跃消息子系统，从而将管理权交给后端
		UActiveMessageSubsystem* ActiveSubsystem = GEditor->GetEditorSubsystem<UActiveMessageSubsystem>();
		if (ActiveSubsystem)
		{
			ActiveSubsystem->RegisterMessageHub(SharedThis(this));
		}
	}

	RefreshVisibility();
}

void SMessageInteractionHub::AddMessageWidget(TSharedRef<SWidget> InWidget)
{
	if (!MessageList.IsValid()) return;
	MessageList->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 10.0f)
	[
		InWidget
	];

		// 自动滚动到最新消息
	ScrollBoxWidget->ScrollToEnd();
	RefreshVisibility();
}

void SMessageInteractionHub::MoveWidgetToBottom(TSharedRef<SWidget> InWidget)
{
	if (!MessageList.IsValid()) return;

	// 1. 尝试查找并移除已存在的槽位
	for (int32 i = 0; i < MessageList->NumSlots(); ++i)
	{
		if (MessageList->GetSlot(i).GetWidget() == InWidget)
		{
			MessageList->RemoveSlot(InWidget);
			break;
		}
	}

	// 2. 无论之前是否存在，都将其添加到底部
	AddMessageWidget(InWidget);
}

void SMessageInteractionHub::RemoveMessageWidget(TSharedRef<SWidget> InWidget)
{
	if (MessageList.IsValid())
	{
		MessageList->RemoveSlot(InWidget);
		RefreshVisibility();
	}
}

void SMessageInteractionHub::ClearMessages()
{
	if (MessageList.IsValid())
	{
		MessageList->ClearChildren();
		RefreshVisibility();
	}
}

void SMessageInteractionHub::RefreshVisibility()
{
	bool bHasVisibleMessages = false;
	if (MessageList.IsValid())
	{
		FChildren* Children = MessageList->GetChildren();
		for (int32 i = 0; i < Children->Num(); ++i)
		{
			if (Children->GetChildAt(i)->GetVisibility().IsVisible())
			{
				bHasVisibleMessages = true;
				break;
			}
		}
	}

	if (bHasVisibleMessages)
	{
		WelcomeWidget->SetVisibility(EVisibility::Collapsed);
		ScrollBoxWidget->SetVisibility(EVisibility::Visible);
	}
	else
	{
		WelcomeWidget->SetVisibility(EVisibility::Visible);
		ScrollBoxWidget->SetVisibility(EVisibility::Collapsed);
	}
}

void SMessageInteractionHub::OnSessionResumed(const FUmgMcpSessionData& SessionData)
{
	ClearMessages();
	for (const FUmgMcpSessionMessage& Msg : SessionData.Messages)
	{
		AddMessageWidget(CreateWidgetFromMessage(Msg));
	}
}

void SMessageInteractionHub::OnMessageAppended(const FUmgMcpSessionMessage& Message)
{
	// 只有非 Agent 的消息（如历史同步的用户消息）在此立即添加。
	// Agent 的消息由 ActiveMessageSubsystem 通过活跃 Widget 动态更新。
	if (Message.Role != EUmgMcpSessionRole::Agent)
	{
		AddMessageWidget(CreateWidgetFromMessage(Message));
	}
}

void SMessageInteractionHub::OnSessionDeleted(const FString& SessionId)
{
	// 重新加载历史列表
	if (WelcomeWidget.IsValid())
	{
		WelcomeWidget->RefreshHistoryList();
	}
}

TSharedRef<SWidget> SMessageInteractionHub::CreateWidgetFromMessage(const FUmgMcpSessionMessage& Message)
{
	switch (Message.Role)
	{
	case EUmgMcpSessionRole::User:
		return SNew(SUserMessageWidget)
			.MessageText(Message.Content)
			.Base64Images(Message.Base64Images);
	case EUmgMcpSessionRole::System:
		return SNew(SSystemNotificationWidget).MessageText(Message.Content);
	case EUmgMcpSessionRole::Agent:
		return SNew(SAgentResponseGroup)
			.AgentName(Message.AgentName)
			.MessageText(Message.Content);
	default:
		return SNew(STextBlock).Text(FText::FromString(TEXT("Unknown Message")));
	}
}
