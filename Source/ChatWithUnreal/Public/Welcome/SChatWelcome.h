// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "FabServer/AIProviders/Zhipu/UmgMcpZhipuConversationHistory.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

DECLARE_DELEGATE_OneParam(FOnUmgMcpSessionSelected, const FString&);

// SChatWelcome：欢迎页，显示历史列表。
class CHATWITHUNREAL_API SChatWelcome : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SChatWelcome) {}
		SLATE_EVENT(FOnUmgMcpSessionSelected, OnSessionSelected)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void RefreshHistoryList();

private:
	// 修复签名：返回 FReply 以匹配 SButton::OnClicked 预期，且统一样式
	FReply OnSessionClicked(FString SessionId);
	EVisibility GetEmptyListVisibility() const;

	FOnUmgMcpSessionSelected OnSessionSelected;
	TSharedPtr<class SVerticalBox> HistoryListBox;
};
