// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#pragma once

#include "CoreMinimal.h"

#include "Styling/SlateBrush.h"

// 前置声明，防止循环依赖
struct FUmgMcpSessionData;
struct FUmgMcpSessionMessage;

// 1. 数据与状态变更的广播（后端发，前端接）
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUmgMcpSessionResumed, const FUmgMcpSessionData&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUmgMcpMessageAppended, const FUmgMcpSessionMessage&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUmgMcpSessionDeleted, const FString& /*SessionId*/);
DECLARE_MULTICAST_DELEGATE(FOnUmgMcpSessionChanged);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUmgMcpInteractionModeChanged, const FString&);
DECLARE_MULTICAST_DELEGATE_FourParams(FOnUmgMcpRequestRestoreMessageWidget, const FString& /*AgentName*/, const FString& /*Content*/, const TArray<FString>& /*Base64Images*/, bool /*bIsUser*/);

// 2. 前端 UI 触发的控制广播（前端发，后端接）
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUmgMcpSessionSelected, const FString& /*SessionId*/);
DECLARE_MULTICAST_DELEGATE(FOnUmgMcpNewConversation);
DECLARE_MULTICAST_DELEGATE(FOnUmgMcpShowHistory);
DECLARE_MULTICAST_DELEGATE(FOnUmgMcpSendClicked);
DECLARE_MULTICAST_DELEGATE(FOnUmgMcpInterruptClicked);

DECLARE_DELEGATE_RetVal(TArray<FString>, FOnUmgMcpGetAvailableAgents);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUmgMcpAtAgentChanged, const FString&);
DECLARE_DELEGATE_RetVal_OneParam(const FSlateBrush*, FOnUmgMcpGetAgentAvatar, const FString&);

/**
 * FUmgMcpDelegates
 * 
 * 顾名思义：在此处单独、集中导出整个 FabServer 系统所有的多播广播事件委托。
 * 前端（ChatWithUnreal）与后端（UmgMcp）通过订阅此处的静态广播来进行完美的端到端逻辑解耦！
 */
class CHATWITHUNREAL_API FUmgMcpDelegates
{
public:
	// 后端 -> 前端
	static FOnUmgMcpSessionResumed OnSessionResumed;
	static FOnUmgMcpMessageAppended OnMessageAppended;
	static FOnUmgMcpSessionDeleted OnSessionDeleted;
	static FOnUmgMcpSessionChanged OnSessionChanged;
	static FOnUmgMcpInteractionModeChanged OnInteractionModeChanged;
	static FOnUmgMcpRequestRestoreMessageWidget OnRequestRestoreMessageWidget;

	// 前端 -> 后端
	static FOnUmgMcpSessionSelected OnSessionSelected;
	static FOnUmgMcpNewConversation OnNewConversation;
	static FOnUmgMcpShowHistory OnShowHistory;
	static FOnUmgMcpSendClicked OnSendClicked;
	static FOnUmgMcpInterruptClicked OnInterruptClicked;

	static FOnUmgMcpGetAvailableAgents OnGetAvailableAgents;
	static FOnUmgMcpAtAgentChanged OnAtAgentChanged;
	static FOnUmgMcpGetAgentAvatar OnGetAgentAvatar;
};
