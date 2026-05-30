// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#include "ChatWithUnrealStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"

TSharedPtr< FSlateStyleSet > FChatWithUnrealStyle::StyleInstance = nullptr;

void FChatWithUnrealStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Realize();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FChatWithUnrealStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FChatWithUnrealStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ChatWithUnrealStyle"));
	return StyleSetName;
}

const ISlateStyle& FChatWithUnrealStyle::Get()
{
	return *StyleInstance;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir(RelativePath, TEXT(".png") ), __VA_ARGS__ )

TSharedRef< FSlateStyleSet > FChatWithUnrealStyle::Realize()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	
	// --- 标准 UE 插件路径获取方式 ---
	TSharedPtr<IPlugin> MyPlugin = IPluginManager::Get().FindPlugin(TEXT("UmgMcp"));
	
	if (MyPlugin.IsValid())
	{
		// 物理根目录：D:\UE5Project\FabUMGMCP\plugins\FabUmgMcp\Resources
		Style->SetContentRoot(MyPlugin->GetBaseDir() / TEXT("Resources"));
	}

	// 统一映射：资源路径自愈
	Style->Set("ChatWithUnreal.PluginIcon", new IMAGE_BRUSH(TEXT("Icon128"), FVector2D(128.0f, 128.0f)));
	Style->Set("ChatWithUnreal.ChatAvatar", new IMAGE_BRUSH(TEXT("Icon/Agent"), FVector2D(40.0f, 40.0f)));

	return Style;
}

#undef IMAGE_BRUSH

void FChatWithUnrealStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}
