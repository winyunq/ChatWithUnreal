// Copyright (c) 2025-2026 Winyunq. All rights reserved.

#include "ChatWithUnreal.h"
#include "UmgMcp.h"
#include "SChatWindow.h"

void FChatWithUnrealModule::StartupModule()
{
	FUmgMcpModule::OnSpawnChatWindow.BindLambda([]()
	{
		return SNew(SChatWindow);
	});
}

void FChatWithUnrealModule::ShutdownModule()
{
	// Put shutdown logic here if needed
}

IMPLEMENT_MODULE(FChatWithUnrealModule, ChatWithUnreal)
