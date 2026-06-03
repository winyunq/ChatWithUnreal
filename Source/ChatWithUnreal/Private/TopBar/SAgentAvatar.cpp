// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#include "SAgentAvatar.h"
#include "ChatWithUnrealStyle.h"
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
	
	const FSlateBrush* TargetBrush = nullptr;
	if (AgentName.Equals(TEXT("Layout"), ESearchCase::IgnoreCase))
	{
		TargetBrush = FChatWithUnrealStyle::Get().GetBrush("ChatWithUnreal.Agent.Layout");
	}
	else if (AgentName.Equals(TEXT("Material"), ESearchCase::IgnoreCase) || AgentName.Equals(TEXT("GlobalMaterial"), ESearchCase::IgnoreCase))
	{
		TargetBrush = FChatWithUnrealStyle::Get().GetBrush("ChatWithUnreal.Agent.Material");
	}
	else if (AgentName.Equals(TEXT("Sequence"), ESearchCase::IgnoreCase) || AgentName.Equals(TEXT("AnimationSequence"), ESearchCase::IgnoreCase))
	{
		TargetBrush = FChatWithUnrealStyle::Get().GetBrush("ChatWithUnreal.Agent.Sequence");
	}
	else if (AgentName.Equals(TEXT("Widget"), ESearchCase::IgnoreCase) || AgentName.Equals(TEXT("Umg"), ESearchCase::IgnoreCase) || AgentName.Equals(TEXT("GlobalUmg"), ESearchCase::IgnoreCase))
	{
		TargetBrush = FChatWithUnrealStyle::Get().GetBrush("ChatWithUnreal.Agent.Widget");
	}
	
	if (!TargetBrush)
	{
		TargetBrush = FChatWithUnrealStyle::Get().GetBrush("ChatWithUnreal.Agent.Agent");
	}

	SourceBrush = TargetBrush;
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
