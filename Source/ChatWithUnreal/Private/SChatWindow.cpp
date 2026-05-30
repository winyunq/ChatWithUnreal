// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#include "SChatWindow.h"
#include "SBottomBar.h"
#include "SMessageInteractionHub.h"
#include "STopBar.h"
#include "SChatWelcome.h"
#include "SChatSendButton.h"
#include "Styling/AppStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Editor.h"

SChatWindow::FOnChatWindowConstructed SChatWindow::OnConstructed;

void SChatWindow::Construct(const FArguments& InArgs)
{
	OnShowHistoryEvent = InArgs._OnShowHistory;
	OnNewConversationEvent = InArgs._OnNewConversation;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.Padding(FMargin(10.0f, 6.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(TopBarWidget, STopBar)
				.OnNewConversation(FSimpleDelegate::CreateSP(this, &SChatWindow::OnNewConversationClicked))
				.OnShowHistory(FSimpleDelegate::CreateSP(this, &SChatWindow::OnShowHistoryClicked))
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(0.0f, 6.0f, 0.0f, 6.0f)
			[
				SAssignNew(MessageHubWidget, SMessageInteractionHub)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(ChatInputWidget, SBottomBar)
			]
		]
	];

	// 动态广播自身，供后端 Subsystem 在运行时动态捕获并进行逻辑绑定
	OnConstructed.Broadcast(SharedThis(this));
}

SChatWindow::~SChatWindow()
{
}

void SChatWindow::OnShowHistoryClicked()
{
	OnShowHistoryEvent.ExecuteIfBound();
}

void SChatWindow::OnNewConversationClicked()
{
	OnNewConversationEvent.ExecuteIfBound();
}

void SChatWindow::OnWelcomeSessionSelected(const FString& SessionId)
{
}
