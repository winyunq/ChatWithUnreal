// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#include "SEditableName.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Styling/AppStyle.h"

void SEditableName::Construct(const FArguments& InArgs)
{
	UserName = InArgs._UserName;
	IsLoggedIn = InArgs._IsLoggedIn;

	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			// 1. 只读文本 (Read-only)
			SNew(STextBlock)
			.Text(this, &SEditableName::GetNameText)
			.Font(FAppStyle::Get().GetFontStyle("NormalFontBold"))
			.Visibility(this, &SEditableName::GetReadOnlyVisibility)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			// 2. 编辑输入框 (Editable)
			SAssignNew(EditableText, SEditableText)
			.Text(this, &SEditableName::GetNameText)
			.Font(FAppStyle::Get().GetFontStyle("NormalFontBold"))
			.OnTextCommitted(this, &SEditableName::OnNameCommitted)
			.Visibility(this, &SEditableName::GetEditableVisibility)
			.MinDesiredWidth(50.0f)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f, 0.0f, 0.0f)
		[
			// 3. 编辑按钮 (Edit Button) - 只有非登录态可见
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.OnClicked(this, &SEditableName::OnEditClicked)
			.Visibility(this, &SEditableName::GetEditButtonVisibility)
			.ToolTipText(FText::FromString(TEXT("Edit Name")))
			[

					SNew(SImage)
					.Image(FAppStyle::Get().GetBrush("Icons.Edit"))
					.ColorAndOpacity(FSlateColor::UseForeground())
				
			]
		]
	];
}

FText SEditableName::GetNameText() const
{
	return FText::FromString(UserName.Get());
}

EVisibility SEditableName::GetReadOnlyVisibility() const
{
	// 登录态始终只读；非登录态下根据 bIsEditing 决定
	if (IsLoggedIn.Get()) return EVisibility::Visible;
	return bIsEditing ? EVisibility::Collapsed : EVisibility::Visible;
}

EVisibility SEditableName::GetEditableVisibility() const
{
	if (IsLoggedIn.Get()) return EVisibility::Collapsed;
	return bIsEditing ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SEditableName::GetEditButtonVisibility() const
{
	// 只有未登录且不在编辑中时，才显示编辑按钮
	return (!IsLoggedIn.Get() && !bIsEditing) ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply SEditableName::OnEditClicked()
{
	bIsEditing = true;
	return FReply::Handled();
}

void SEditableName::OnNameCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		// 模拟保存逻辑（实际应通过委托传回）
		bIsEditing = false;
	}
	else if (CommitType == ETextCommit::Default)
	{
		bIsEditing = false;
	}
}
