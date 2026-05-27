// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#include "SAgentAvatar.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"

namespace
{
	FText GetLocText(const FString& English, const FString& Chinese)
	{
		const FString Culture = FInternationalization::Get().GetCurrentCulture()->GetName();
		return FText::FromString(Culture.StartsWith(TEXT("zh")) ? Chinese : English);
	}
}

void SAgentAvatar::Construct(const FArguments& InArgs)
{
	AgentName = InArgs._AgentName;
	SAvatar::Construct(SAvatar::FArguments());
}

TSharedRef<SWidget> SAgentAvatar::OnGetMenuContent()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection("AgentActions", GetLocText(TEXT("Agent Actions"), TEXT("Agent 操作")));
	{
		const FString MentionText = FString::Printf(TEXT("@%s"), *AgentName);
		MenuBuilder.AddMenuEntry(
			FText::Format(GetLocText(TEXT("Mention {0}"), TEXT("提及 {0}")), FText::FromString(MentionText)),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([MentionText]() {
				// TODO: 插入 @ 到输入框
			}))
		);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}
