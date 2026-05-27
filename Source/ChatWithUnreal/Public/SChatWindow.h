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
	SLATE_BEGIN_ARGS(SChatWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SChatWindow();

private:
	void OnShowHistoryClicked();
	void OnNewConversationClicked();
	void OnWelcomeSessionSelected(const FString& SessionId);

	TSharedPtr<STopBar> TopBarWidget;
	TSharedPtr<SMessageInteractionHub> MessageHubWidget;
	TSharedPtr<SBottomBar> ChatInputWidget;
};
