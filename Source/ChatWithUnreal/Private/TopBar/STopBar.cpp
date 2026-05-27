// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#include "STopBar.h"

#include "SUserAvatar.h"
#include "SEditableName.h"
#include "FabServer/ChatSystem/UmgMcpActiveMessageSubsystem.h"
#include "FabServer/Authentication/UmgMcpAuthenticationSubsystem.h"
#include "Editor.h"
#include "Styling/AppStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"

void STopBar::Construct(const FArguments& InArgs)
{
	OnNewConversationDelegate = InArgs._OnNewConversation;
	OnShowHistoryDelegate = InArgs._OnShowHistory;

	LocalUserName = TEXT("Guest");
	bIsLoggedIn = false;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.Padding(FMargin(10.0f, 5.0f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SAssignNew(AvatarWidget, SUserAvatar)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(10.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SEditableName)
				.UserName(this, &STopBar::GetLocalUserName)
				.IsLoggedIn(this, &STopBar::GetIsLoggedIn)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SSpacer)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.OnClicked(this, &STopBar::OnShowHistoryClicked)
				.ToolTipText(this, &STopBar::GetHistoryToolTip)
				[
					SNew(SImage).Image(FAppStyle::Get().GetBrush("Icons.Layout"))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.OnClicked(this, &STopBar::OnNewConversationClicked)
				.ToolTipText(this, &STopBar::GetNewConvToolTip)
				[
						SNew(SImage).Image(FAppStyle::Get().GetBrush("Icons.Plus"))
					
				]
			]
		]
	];
}

FReply STopBar::OnNewConversationClicked()
{
	// 1. 仅保留形式通知用于 UI 页面切换
	OnNewConversationDelegate.ExecuteIfBound();
	return FReply::Handled();
}

FReply STopBar::OnShowHistoryClicked()
{
	OnShowHistoryDelegate.ExecuteIfBound();
	return FReply::Handled();
}

// ... 辅助函数省略 ...
FString STopBar::GetLocalUserName() const
{
	if (GEditor)
	{
		if (UAuthenticationSubsystem* AuthSys = GEditor->GetEditorSubsystem<UAuthenticationSubsystem>())
		{
			return AuthSys->GetActiveUserDisplayName();
		}
	}
	return NSLOCTEXT("UmgMcp", "LocalUser", "Local User").ToString();
}

bool STopBar::GetIsLoggedIn() const
{
	if (GEditor)
	{
		if (UAuthenticationSubsystem* AuthSys = GEditor->GetEditorSubsystem<UAuthenticationSubsystem>())
		{
			return AuthSys->IsAnyAccountLoggedIn();
		}
	}
	return bIsLoggedIn;
}
FText STopBar::GetHistoryToolTip() const { return FText::FromString(TEXT("History")); }
FText STopBar::GetNewConvToolTip() const { return FText::FromString(TEXT("New Conversation")); }