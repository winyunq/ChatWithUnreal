// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

DECLARE_DELEGATE_OneParam(FOnImageTagErased, int32 /*ErasedIndex*/);
DECLARE_DELEGATE_OneParam(FOnFilesDropped, const TArray<FString>& /*Files*/);

/**
 * SChatInput：原子控件，仅负责多行文本输入。
 */
class CHATWITHUNREAL_API SChatInput : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SChatInput) {}
		SLATE_EVENT(FSimpleDelegate, OnSendShortcutTriggered)
		SLATE_EVENT(FSimpleDelegate, OnPasteShortcutTriggered)
		SLATE_EVENT(FOnImageTagErased, OnImageTagErased)
		SLATE_EVENT(FSimpleDelegate, OnAllImageTagsErased)
		SLATE_EVENT(FOnFilesDropped, OnFilesDropped)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FText GetText() const;
	void SetText(const FText& InText);
	void ClearText();
	
	void RemoveImageTag(int32 TargetIndex);
	void InsertImageTag(const FString& InImageId);

public:
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;

private:
	FReply OnInputKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);
	void HandleTextChanged(const FText& NewText);

	TSharedPtr<class SMultiLineEditableTextBox> InputTextBox;
	FString LastText;
	bool bIsUpdatingText = false;

	FSimpleDelegate OnSendShortcutTriggeredEvent;
	FSimpleDelegate OnPasteShortcutTriggeredEvent;
	FOnImageTagErased OnImageTagErasedEvent;
	FSimpleDelegate OnAllImageTagsErasedEvent;
	FOnFilesDropped OnFilesDroppedEvent;
};
