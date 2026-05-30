// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SChatInput;
class SAbilitiesSelector;
class SInteractionModeSelector;
class SToolModeSelector;
class SChatSendButton;
class SQuotaBar;
class SAttachmentList;

DECLARE_DELEGATE_OneParam(FOnBottomBarInteractionModeChanged, const FString& /*NewMode*/);
DECLARE_DELEGATE_OneParam(FOnBottomBarToolModeChanged, const FString& /*NewTool*/);

/**
 * SBottomBar
 */
class CHATWITHUNREAL_API SBottomBar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBottomBar) {}
		SLATE_EVENT(FSimpleDelegate, OnSendClicked)
		SLATE_EVENT(FOnBottomBarInteractionModeChanged, OnInteractionModeChanged)
		SLATE_EVENT(FOnBottomBarToolModeChanged, OnToolModeChanged)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// 原子微控件
	TSharedPtr<SAbilitiesSelector> AbilitiesSelector;
	TSharedPtr<SChatInput> ChatInput;
	TSharedPtr<SInteractionModeSelector> InteractionModeSelector;
	TSharedPtr<SToolModeSelector> ToolModeSelector;
	TSharedPtr<SChatSendButton> SendButton;
	TSharedPtr<SAttachmentList> AttachmentList;
	TSharedPtr<SQuotaBar> QuotaBar;

private:
	void OnInteractionModeChanged(const FString& NewMode);
	void OnToolModeChanged(const FString& NewTool);
	FString GetInteractionMode() const;
	void OnChatInputSendRequested();
	void OnPasteImageFromClipboard();
	FReply OnAddAttachmentClicked();
	void OnFilesDropped(const TArray<FString>& Files);

	FSimpleDelegate OnSendClickedEvent;
	FOnBottomBarInteractionModeChanged OnInteractionModeChangedEvent;
	FOnBottomBarToolModeChanged OnToolModeChangedEvent;
};
