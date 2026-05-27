// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Dom/JsonObject.h"

class FUmgMcpAgent;
class SMcpToolItem;

/**
 * SMcpApprovalQueue: MCP 审批队列控件。
 * 实现垂直列表、单步推进与按钮焦点滑动。
 */
class CHATWITHUNREAL_API SMcpApprovalQueue : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMcpApprovalQueue) {}
		SLATE_ARGUMENT(TWeakPtr<FUmgMcpAgent>, WaitingAgent)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** 添加一个工具请求到队列末尾 */
	void AddToolRequest(const FString& ToolName, const FString& ArgumentsJson);

	/** 呈现第一个审批按钮（定界启动） */
	void PresentFirstApprovalButtons();

	/** 判定队列是否为空（逻辑上是否有请求） */
	bool IsQueueEmpty() const { return ToolItems.Num() == 0; }

private:
	/** 当前焦点索引（待处理的第一个 Item） */
	int32 CurrentFocusIndex = 0;

	/** 工具请求项列表 */
	TArray<TSharedPtr<SMcpToolItem>> ToolItems;

	/** 已收集的执行结果 */
	TArray<TSharedPtr<FJsonObject>> CollectedResults;

	/** 容器布局 */
	TSharedPtr<class SVerticalBox> ListContainer;

	/** 回调目标 */
	TWeakPtr<FUmgMcpAgent> WaitingAgent;

	// --- 交互执行节点 ---
	/** 为当前焦点 Item 构造并显示按钮 */
	void RenderActiveButtons();

	/** 执行动作：调用后台 MCP 命令 */
	FReply OnAcceptClicked();
	FReply OnRejectClicked();

	/** 单步推进：处理完一个后的逻辑切割 */
	void AdvanceFocus();
};
