#include "Messages/SAgentResponseGroup/SMcpApprovalQueue.h"
#include "Messages/SAgentResponseGroup/SMcpToolItem.h"
#include "FabServer/Agent/UmgMcpAgent.h"
#include "FabServer/FabServerHttpClient.h"
#include "FabServer/ChatSystem/UmgMcpActiveMessageSubsystem.h"
#include "FabServer/AIProviders/UmgMcpAiSubsystem.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/AppStyle.h"
#include "Editor.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"

#include "Widgets/Layout/SBorder.h"

#define LOCTEXT_NAMESPACE "SMcpApprovalQueue"

void SMcpApprovalQueue::Construct(const FArguments& InArgs)
{
	WaitingAgent = InArgs._WaitingAgent;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("DetailsView.CategoryMiddle"))
		.BorderBackgroundColor(FLinearColor(0.11f, 0.14f, 0.18f, 1.0f))
		.Padding(FMargin(10.0f, 8.0f))
		[
			SAssignNew(ListContainer, SVerticalBox)
		]
	];
}

void SMcpApprovalQueue::AddToolRequest(const FString& ToolName, const FString& ArgumentsJson)
{
	if (ListContainer.IsValid())
	{
		TSharedPtr<SMcpToolItem> NewItem = SNew(SMcpToolItem)
			.ToolName(ToolName)
			.ArgumentsJson(ArgumentsJson);
		
		ToolItems.Add(NewItem);
		ListContainer->AddSlot()
		.AutoHeight()
		[
			NewItem.ToSharedRef()
		];
	}
}

void SMcpApprovalQueue::PresentFirstApprovalButtons()
{
	CurrentFocusIndex = 0;
	RenderActiveButtons();
}

void SMcpApprovalQueue::RenderActiveButtons()
{
	if (!ToolItems.IsValidIndex(CurrentFocusIndex))
	{
		// 队列已全部处理完，回调 Agent
		if (WaitingAgent.IsValid())
		{
			WaitingAgent.Pin()->OnMcpTaskFinished(CollectedResults);
		}
		return;
	}

	// 在当前 Item 所在的 Slot 之后插入按钮组
	// 注意：更严谨的做法是 Item 控件本身预留一个按钮容器 Slot
	ListContainer->AddSlot()
	.AutoHeight()
	.Padding(16.0f, 4.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("Accept", "Accept"))
			.ButtonStyle(FAppStyle::Get(), "PrimaryButton")
			.OnClicked(this, &SMcpApprovalQueue::OnAcceptClicked)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(8.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Reject", "Reject"))
			.OnClicked(this, &SMcpApprovalQueue::OnRejectClicked)
		]
	];
}

FReply SMcpApprovalQueue::OnAcceptClicked()
{
	if (!ToolItems.IsValidIndex(CurrentFocusIndex)) return FReply::Handled();

	TSharedPtr<SMcpToolItem> ActiveItem = ToolItems[CurrentFocusIndex];
	ActiveItem->SetStatus(SMcpToolItem::EToolStatus::Running);

	// 1. 更新全局状态为 MCP Running
	if (UActiveMessageSubsystem* ActiveSubsystem = GEditor ? GEditor->GetEditorSubsystem<UActiveMessageSubsystem>() : nullptr)
	{
		FString AgentName = WaitingAgent.IsValid() ? WaitingAgent.Pin()->Name : TEXT("Agent");
		ActiveSubsystem->UpdateActiveMessageMeta(AgentName, LOCTEXT("McpRunning", "MCP Running..."), true);
	}

	// 2. 构造参数对象
	TSharedPtr<FJsonObject> Args;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ActiveItem->GetArgumentsJson());
	FJsonSerializer::Deserialize(Reader, Args);

	// 3. 异步调用后台命令
	// 这里简化处理，直接获取结果。实际生产中应当支持 Http 回调
	FString ResultString = FFabServerHttpClient::ExecuteTool(ActiveItem->GetToolName(), Args);

	// 4. 更新 Item 状态与结果集
	ActiveItem->SetStatus(SMcpToolItem::EToolStatus::Success, TEXT(""), ResultString);

	// 从 WaitingAgent 的 ContextHistory 最后一条 assistant 消息的 tool_calls 里，
	// 按 function.name 匹配查出 AI Round 1 分配的原始 tool_call_id。
	FString ResolvedToolCallId;
	if (WaitingAgent.IsValid())
	{
		TSharedPtr<FUmgMcpAgent> Agent = WaitingAgent.Pin();
		const FString& MatchName = ActiveItem->GetToolName();
		// 从尾部向前找最近一条有 tool_calls 的 assistant 消息
		for (int32 i = Agent->ContextHistory.Num() - 1; i >= 0; --i)
		{
			const TSharedPtr<FJsonObject>& Msg = Agent->ContextHistory[i];
			if (!Msg.IsValid()) continue;
			FString Role;
			Msg->TryGetStringField(TEXT("role"), Role);
			if (Role != TEXT("assistant")) continue;

			const TArray<TSharedPtr<FJsonValue>>* ToolCallsArr;
			if (!Msg->TryGetArrayField(TEXT("tool_calls"), ToolCallsArr)) continue;

			for (const auto& TCVal : *ToolCallsArr)
			{
				TSharedPtr<FJsonObject> TC = TCVal->AsObject();
				if (!TC.IsValid()) continue;

				// 尝试从 function.name 或直接 name 字段匹配
				FString FuncName;
				const TSharedPtr<FJsonObject>* FuncPtr;
				if (TC->TryGetObjectField(TEXT("function"), FuncPtr) && FuncPtr)
				{
					(*FuncPtr)->TryGetStringField(TEXT("name"), FuncName);
				}
				else
				{
					TC->TryGetStringField(TEXT("name"), FuncName);
				}

				if (FuncName == MatchName)
				{
					TC->TryGetStringField(TEXT("id"), ResolvedToolCallId);
					break;
				}
			}
			if (!ResolvedToolCallId.IsEmpty()) break;
		}
	}
	// 兜底：若历史中找不到对应 id，退回到基于工具名的稳定伪 ID（不再使用纯随机 GUID）
	if (ResolvedToolCallId.IsEmpty())
	{
		ResolvedToolCallId = TEXT("call_") + ActiveItem->GetToolName();
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("role"), TEXT("tool"));
	ResultObj->SetStringField(TEXT("tool_call_id"), ResolvedToolCallId);
	ResultObj->SetStringField(TEXT("name"), ActiveItem->GetToolName());
	ResultObj->SetStringField(TEXT("content"), ResultString);
	CollectedResults.Add(ResultObj);

	// 5. 推进焦点
	AdvanceFocus();
	return FReply::Handled();
}

FReply SMcpApprovalQueue::OnRejectClicked()
{
	if (!ToolItems.IsValidIndex(CurrentFocusIndex)) return FReply::Handled();

	ToolItems[CurrentFocusIndex]->SetStatus(SMcpToolItem::EToolStatus::Rejected);
	
	// 记录一个被拒绝的状态回给 AI
	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("role"), TEXT("tool"));
	ResultObj->SetStringField(TEXT("content"), TEXT("User rejected this tool call."));
	CollectedResults.Add(ResultObj);

	AdvanceFocus();
	return FReply::Handled();
}

void SMcpApprovalQueue::AdvanceFocus()
{
	// 移除最后添加的那个按钮组 Slot (即末尾项)
	ListContainer->RemoveSlot(ListContainer->GetChildren()->GetChildAt(ListContainer->GetChildren()->Num() - 1));

	CurrentFocusIndex++;

	// 恢复状态为 Waiting (如果是最后一个则后续会由 Agent 处理)
	if (UActiveMessageSubsystem* ActiveSubsystem = GEditor ? GEditor->GetEditorSubsystem<UActiveMessageSubsystem>() : nullptr)
	{
		FString AgentName = WaitingAgent.IsValid() ? WaitingAgent.Pin()->Name : TEXT("Agent");
		ActiveSubsystem->UpdateActiveMessageMeta(AgentName, LOCTEXT("WaitingMcp", "Waiting MCP..."), false);
	}

	RenderActiveButtons();
}

#undef LOCTEXT_NAMESPACE
