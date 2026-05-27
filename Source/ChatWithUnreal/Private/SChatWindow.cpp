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
#include "FabServer/ChatSystem/UmgMcpActiveMessageSubsystem.h"
#include "FabServer/ChatSystem/UmgMcpSessionManagerSubsystem.h"
#include "FabServer/AIProviders/UmgMcpAiSubsystem.h"
#include "FabServer/AIProviders/Local/UmgMcpLiteRtLmAiProvider.h"

void SChatWindow::Construct(const FArguments& InArgs)
{
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
}

SChatWindow::~SChatWindow()
{
}

void SChatWindow::OnShowHistoryClicked()
{
	if (GEditor)
	{
		if (auto* Subsystem = GEditor->GetEditorSubsystem<UActiveMessageSubsystem>())
		{
			if (Subsystem->IsGenerating()) return;
		}
	}

	if (MessageHubWidget.IsValid())
	{
		MessageHubWidget->ClearMessages();
	}

	if (GEditor)
	{
		if (USessionManagerSubsystem* SessionSubsystem = GEditor->GetEditorSubsystem<USessionManagerSubsystem>())
		{
			SessionSubsystem->ClearActiveSession();
		}
	}
}

void SChatWindow::OnNewConversationClicked()
{
	if (GEditor)
	{
		if (auto* Subsystem = GEditor->GetEditorSubsystem<UActiveMessageSubsystem>())
		{
			if (Subsystem->IsGenerating()) return;
		}
	}

	if (MessageHubWidget.IsValid())
	{
		MessageHubWidget->ClearMessages();
	}

	if (GEditor)
	{
		if (USessionManagerSubsystem* SessionSubsystem = GEditor->GetEditorSubsystem<USessionManagerSubsystem>())
		{
			SessionSubsystem->ClearActiveSession();
		}

		if (UActiveMessageSubsystem* ActiveSubsystem = GEditor->GetEditorSubsystem<UActiveMessageSubsystem>())
		{
			ActiveSubsystem->StartNewChatMessage();
		}
	}
}

void SChatWindow::OnWelcomeSessionSelected(const FString& SessionId)
{
}
