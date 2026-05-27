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

/**
 * SBottomBar
 */
class CHATWITHUNREAL_API SBottomBar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBottomBar) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// 原子微控件 (Public 为指挥官提供直达访问)
	TSharedPtr<SAbilitiesSelector> AbilitiesSelector;
	TSharedPtr<SChatInput> ChatInput;
	TSharedPtr<SInteractionModeSelector> InteractionModeSelector;
	TSharedPtr<SToolModeSelector> ToolModeSelector;
	TSharedPtr<SChatSendButton> SendButton;
	TSharedPtr<class SAttachmentList> AttachmentList;
	TSharedPtr<SQuotaBar> QuotaBar;

private:
	// 事件回调
	void OnInteractionModeChanged(const FString& NewMode);
	void OnToolModeChanged(const FString& NewTool);
	FReply OnSendClicked();
	void OnChatInputSendRequested();
	void OnChatInputPasteImage(const TArray<uint8>& ImageData, int32 Width, int32 Height);
	FReply OnAddAttachmentClicked();
};
