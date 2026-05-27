// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

// 能力选择原子控件（下拉按钮形式，与 ToolModeSelector 一致）
class CHATWITHUNREAL_API SAbilitiesSelector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAbilitiesSelector) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SAbilitiesSelector();

	EVisibility GetVisibilityBasedOnMode() const;

private:
	TSharedRef<SWidget> CreateAbilitiesMenuContent();
	FText GetDisplayAlphaText() const;

	TSharedPtr<class SComboButton> ComboButton;
};
