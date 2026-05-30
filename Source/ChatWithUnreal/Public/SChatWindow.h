// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class STopBar;
class SChatWelcome;
class SMessageInteractionHub;
class SBottomBar;

class CHATWITHUNREAL_API SChatWindow : public SCompoundWidget
{
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnChatWindowConstructed, TSharedRef<SChatWindow>);
	static FOnChatWindowConstructed OnConstructed;

public:
	SLATE_BEGIN_ARGS(SChatWindow) {}
		SLATE_EVENT(FSimpleDelegate, OnShowHistory)
		SLATE_EVENT(FSimpleDelegate, OnNewConversation)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SChatWindow();

	TSharedPtr<SMessageInteractionHub> GetMessageHub() const { return MessageHubWidget; }
	TSharedPtr<SBottomBar> GetBottomBar() const { return ChatInputWidget; }
	TSharedPtr<STopBar> GetTopBar() const { return TopBarWidget; }

	FSimpleDelegate OnShowHistory;
	FSimpleDelegate OnNewConversation;

private:
	void OnShowHistoryClicked();
	void OnNewConversationClicked();
	void OnWelcomeSessionSelected(const FString& SessionId);

	TSharedPtr<STopBar> TopBarWidget;
	TSharedPtr<SMessageInteractionHub> MessageHubWidget;
	TSharedPtr<SBottomBar> ChatInputWidget;

	FSimpleDelegate OnShowHistoryEvent;
	FSimpleDelegate OnNewConversationEvent;
};
