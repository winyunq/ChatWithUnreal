// Copyright (c) 2025-2026 Winyunq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

struct FSlateDynamicImageBrush;

DECLARE_DELEGATE_OneParam(FOnAttachmentAdded, const FString& /*ImageId*/);
DECLARE_DELEGATE_OneParam(FOnAttachmentRemoved, const FString& /*ImageId*/);

/**
 * FUmgMcpAttachmentItem
 * 附件项结构：绑定了特定的控制 ID、原始 Base64 数据与用于预览的动态 Slate 刷子。
 */
struct FUmgMcpAttachmentItem
{
	FString ImageId;
	FString Base64Data;
	TSharedPtr<FSlateDynamicImageBrush> SlateBrush;
};

/**
 * SAttachmentList
 * 附件列表控件：用于展示和管理当前消息准备发送的图片实质。
 */
class CHATWITHUNREAL_API SAttachmentList : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAttachmentList) {}
		SLATE_EVENT(FOnAttachmentAdded, OnAttachmentAdded)
		SLATE_EVENT(FOnAttachmentRemoved, OnAttachmentRemoved)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** 获取当前所有附件的 Base64 数据 */
	TArray<FString> GetBase64Images() const;

	/** 获取当前所有附件项 */
	const TArray<FUmgMcpAttachmentItem>& GetAttachmentItems() const { return AttachedImages; }

	/** 获取附件数量 */
	int32 GetAttachmentCount() const { return AttachedImages.Num(); }

	/** 清空附件 */
	void ClearAttachments();

	/** 手动添加附件 (Base64) */
	void AddAttachment(const FString& InBase64, const FString& InImageId = TEXT(""));

	/** 根据 ID 删除附件项（用于文本删除联动） */
	void RemoveAttachmentById(const FString& InImageId);

private:
	/** 刷新 UI */
	void RefreshList();

	/** 附件数据列表 */
	TArray<FUmgMcpAttachmentItem> AttachedImages;

	/** 列表容器 */
	TSharedPtr<SHorizontalBox> ListContainer;

	FOnAttachmentAdded OnAttachmentAddedEvent;
	FOnAttachmentRemoved OnAttachmentRemovedEvent;
	int32 ImageIndexCounter = 0;
};
