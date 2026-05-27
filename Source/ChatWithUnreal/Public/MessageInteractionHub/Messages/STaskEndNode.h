// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "FabServer/ChatSystem/UmgMcpTaskSubsystem.h"
#include "Widgets/SCompoundWidget.h"

class CHATWITHUNREAL_API STaskEndNode : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STaskEndNode) {}
		SLATE_ARGUMENT(FUmgMcpTaskRecord, TaskRecord)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
