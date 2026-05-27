// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Styling/SlateBrush.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 * SAvatar: 头像基类。
 * 负责圆形画刷渲染和 SComboButton 的点击弹出响应。
 */
class CHATWITHUNREAL_API SAvatar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAvatar)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// 强制子类实现菜单内容
	virtual TSharedRef<SWidget> OnGetMenuContent() = 0;

protected:
	const FSlateBrush* GetRoundedAvatarBrush() const;
	
	// 交互动画计算
	// FSlateRenderTransform GetAvatarRenderTransform() const;

	const float AvatarSize = 40.0f;
	const FSlateBrush* SourceBrush = nullptr;

	TSharedPtr<class SComboButton> ComboButton;
	mutable TSharedPtr<struct FSlateRoundedBoxBrush> CachedRoundedAvatarBrush;
};
