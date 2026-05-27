// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#include "SAvatar.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/AppStyle.h"
#include "FabServer/UmgMcpStyle.h"
#include "Styling/StyleColors.h"

void SAvatar::Construct(const FArguments& InArgs)
{
	SourceBrush = FUmgMcpStyle::Get().GetBrush("UmgMcp.ChatAvatar");

	ChildSlot
	[
		SAssignNew(ComboButton, SComboButton)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.HasDownArrow(false)
		.ContentPadding(0.0f)
		.OnGetMenuContent(this, &SAvatar::OnGetMenuContent)
		.ButtonContent()
		[
			SNew(SBox)
			.WidthOverride(AvatarSize)
			.HeightOverride(AvatarSize)
			[
				SNew(SImage)
				.Image(this, &SAvatar::GetRoundedAvatarBrush)
			]
		]
	];
}


const FSlateBrush* SAvatar::GetRoundedAvatarBrush() const
{
	const FSlateBrush* EffectiveSource = SourceBrush;
	if (!EffectiveSource)
	{
		EffectiveSource = FAppStyle::Get().GetBrush("Icons.User");
	}

	if (!CachedRoundedAvatarBrush.IsValid())
	{
		CachedRoundedAvatarBrush = MakeShared<FSlateRoundedBoxBrush>(
			EffectiveSource->GetResourceName(),
			FLinearColor::White,
			FVector4(AvatarSize * 0.5f, AvatarSize * 0.5f, AvatarSize * 0.5f, AvatarSize * 0.5f),
			FLinearColor::Transparent,
			0.0f,
			FVector2D(AvatarSize, AvatarSize)
		);
	}

	return CachedRoundedAvatarBrush.Get();
}
