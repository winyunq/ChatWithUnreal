// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

DECLARE_DELEGATE_OneParam(FOnImageTagErased, int32 /*ErasedIndex*/);
DECLARE_DELEGATE_OneParam(FOnFilesDropped, const TArray<FString>& /*Files*/);
DECLARE_DELEGATE_RetVal(FReply, FOnPasteShortcutTriggered);

/**
 * SChatInput：原子控件，仅负责多行文本输入。
 */
class CHATWITHUNREAL_API SChatInput : public SCompoundWidget
{
public:
	static TWeakPtr<SChatInput> Instance;

public:
	SLATE_BEGIN_ARGS(SChatInput) {}
		SLATE_EVENT(FSimpleDelegate, OnSendShortcutTriggered)
		SLATE_EVENT(FOnPasteShortcutTriggered, OnPasteShortcutTriggered)
		SLATE_EVENT(FOnFilesDropped, OnFilesDropped)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FText GetText() const;
	void SetText(const FText& InText);
	void ClearText();
	
	void RemoveImageTag(int32 TargetIndex);
	void InsertImageTag(const FString& InImageId);

	void SetAttachmentList(TSharedPtr<class SAttachmentList> InList);

	FString GetActiveAtAgent() const { return ActiveAtAgent; }

public:
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;

private:
	FReply OnInputKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);
	void HandleTextChanged(const FText& NewText);

	TSharedPtr<class SMultiLineEditableTextBox> InputTextBox;
	FString LastText;
	bool bIsUpdatingText = false;

	TWeakPtr<class SAttachmentList> AttachmentListWidget;

	TSharedPtr<class SMenuAnchor> AgentMenuAnchor;
	TSharedPtr<class SListView<TSharedPtr<FString>>> AgentListView;
	TArray<TSharedPtr<FString>> AgentSuggestions;
	FString ActiveAtAgent;

	TSharedRef<class SWidget> OnGenerateAgentMenu();
	void FilterAgentSuggestions(const FString& FilterText);
	void OnAgentSelected(TSharedPtr<FString> SelectedAgent, ESelectInfo::Type SelectInfo);

	FSimpleDelegate OnSendShortcutTriggeredEvent;
	FOnPasteShortcutTriggered OnPasteShortcutTriggeredEvent;
	FOnFilesDropped OnFilesDroppedEvent;
};
