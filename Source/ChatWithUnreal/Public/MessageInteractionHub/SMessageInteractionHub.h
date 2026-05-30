// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SScrollBox;
class SVerticalBox;
class SChatWelcome;
class SAgentResponseGroup;

DECLARE_DELEGATE_OneParam(FOnMcpSessionSelected, const FString& /*SessionId*/);
DECLARE_DELEGATE_OneParam(FOnMcpSessionDeleted, const FString& /*SessionId*/);

/**
 * SMessageInteractionHub：交互中枢。
 * 负责在欢迎页与消息列表之间切换，并暴露对话重演接口供后端驱动。
 */
class CHATWITHUNREAL_API SMessageInteractionHub : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMessageInteractionHub) {}
		SLATE_EVENT(FOnMcpSessionSelected, OnSessionSelected)
		SLATE_EVENT(FOnMcpSessionDeleted, OnSessionDeleted)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** --- 对话重演协议 (BE -> FE) --- */

	/** 开始一条新的 Agent 响应气泡（历史或直播均走此路） */
	void StartAgentResponse(const FString& AgentName);

	/** 为当前活跃气泡追加内容 */
	void AppendActiveText(const FString& Content);

	/** 设置当前活跃气泡的状态 */
	void SetActiveStatus(const FText& Status, bool bShowSpinner, const FLinearColor& Color = FLinearColor::White);

	/** 追加用户消息（历史或直播均走此路） */
	void AddUserMessage(const FString& Content, const TArray<FString>& Base64Images);

	/** 追加系统通知 */
	void AddSystemNotification(const FString& Content);

	/** 恢复会话起始状态：清空并准备显示历史 */
	void OnSessionResumed();

	/** 清空屏幕 */
	void ClearMessages();

	/** 刷新布局显隐 */
	void RefreshVisibility();

	/** 向列表追加一个原始 Widget（底层接口，尽量由内部使用） */
	void AddMessageWidget(TSharedRef<SWidget> InWidget);
	void MoveWidgetToBottom(TSharedRef<SWidget> InWidget);
	void RemoveMessageWidget(TSharedRef<SWidget> InWidget);

	TSharedPtr<SChatWelcome> WelcomeWidget;
	FOnMcpSessionSelected OnSessionSelectedEvent;
	FOnMcpSessionDeleted OnSessionDeletedEvent;

private:
	EVisibility GetWelcomeVisibility() const;

	TSharedPtr<SVerticalBox> MessageList;
	TSharedPtr<SScrollBox> ScrollBoxWidget;

	/** 当前正在接受后端灌入数据的活跃气泡 */
	TSharedPtr<SAgentResponseGroup> ActiveResponseGroup;
};
