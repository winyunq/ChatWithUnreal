// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

DECLARE_DELEGATE(FOnAtAgentMessageClearClicked);
DECLARE_DELEGATE_OneParam(FOnAgentSuggestionSelected, const FString& /*SelectedAgentName*/);
DECLARE_DELEGATE_RetVal_OneParam(const struct FSlateBrush*, FOnGetAgentAvatar, const FString&);

/**
 * SAtAgentMessage: 完全自治的 @智能体 状态展示与清除组件。
 */
class CHATWITHUNREAL_API SAtAgentMessage : public SCompoundWidget
{
public:
	static TWeakPtr<SAtAgentMessage> Instance;

public:
	SLATE_BEGIN_ARGS(SAtAgentMessage) {}
		SLATE_EVENT(FOnAtAgentMessageClearClicked, OnClearClicked)
		SLATE_EVENT(FOnAgentSuggestionSelected, OnAgentSelected)
		SLATE_EVENT(FOnGetAgentAvatar, OnGetAgentAvatar)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void UpdateAgent(const FString& InAgentName, const struct FSlateBrush* InBrush);
	FString GetAgentName() const { return AgentName; }

	void ShowSuggestionsMenu(const TArray<FString>& InSuggestions);
	void CloseSuggestionsMenu();

	FOnAtAgentMessageClearClicked OnClearClickedEvent;
	FOnAgentSuggestionSelected OnAgentSelectedEvent;
	FOnGetAgentAvatar OnGetAgentAvatarEvent;

private:
	FReply OnClearBtnClicked();

	TSharedRef<SWidget> OnGenerateAgentMenu();
	TSharedRef<ITableRow> OnGenerateAgentRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnAgentSelected(TSharedPtr<FString> SelectedAgent, ESelectInfo::Type SelectInfo);

	TSharedPtr<class SMenuAnchor> AgentMenuAnchor;
	TSharedPtr<class SListView<TSharedPtr<FString>>> AgentListView;
	TArray<TSharedPtr<FString>> AgentSuggestions;

	FString AgentName;
	const struct FSlateBrush* AgentBrush = nullptr;
};
