// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "FabServer/ChatSystem/UmgMcpTaskSubsystem.h"
#include "Widgets/SCompoundWidget.h"

class CHATWITHUNREAL_API STaskBeginNode : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STaskBeginNode) {}
		SLATE_ARGUMENT(FString, InitiatorAgent)
		SLATE_ARGUMENT(FString, ReceiverAgent)
		SLATE_ARGUMENT(FString, TargetAsset)
		SLATE_ARGUMENT(TArray<FString>, Items)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FString InitiatorAgent;
	FString ReceiverAgent;
	FString TargetAsset;
	TArray<FString> Items;
	TArray<FString> ItemFeedbacks;

	// TSharedRef<SWidget> BuildAgentBadge(const FString& InAgentName);
	FString FormatTaskItemSingleLine(const FString& InText);
};
