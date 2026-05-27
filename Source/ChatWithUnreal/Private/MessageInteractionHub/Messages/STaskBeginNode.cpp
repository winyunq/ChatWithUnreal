// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#include "Messages/STaskBeginNode.h"
#include "SAgentAvatar.h"
#include "Styling/AppStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"

namespace 
{
	FString GetLocString(const FString& English, const FString& Chinese)
	{
		const FString Culture = FInternationalization::Get().GetCurrentCulture()->GetTwoLetterISOLanguageName();
		return Culture.StartsWith(TEXT("zh")) ? Chinese : English;
	}

	TSharedRef<SWidget> BuildAgentBadge(const FString& InAgentName)
	{
		return SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.22f, 1.0f))
			.Padding(FMargin(4.0f, 4.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(24.0f)
					.HeightOverride(24.0f)
					[
						SNew(SAgentAvatar)
						.AgentName(InAgentName)
					]
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(8.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(InAgentName))
					.ColorAndOpacity(FLinearColor(0.85f, 0.85f, 0.85f, 1.0f))
				]
			];
	}
}

void STaskBeginNode::Construct(const FArguments& InArgs)
{
	InitiatorAgent = InArgs._InitiatorAgent;
	ReceiverAgent = InArgs._ReceiverAgent;
	TargetAsset = InArgs._TargetAsset;
	Items = InArgs._Items;
	ItemFeedbacks.Init(FString(), Items.Num());

	TSharedPtr<SVerticalBox> ItemsHost;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
		.Padding(FMargin(10.0f, 8.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					BuildAgentBadge(InitiatorAgent)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				.Padding(8.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("→")))
						.Font(FAppStyle::Get().GetFontStyle("SmallFontBold"))
						.ColorAndOpacity(FLinearColor(0.75f, 0.75f, 0.75f, 1.0f))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					.Padding(6.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("Target: %s"), TargetAsset.IsEmpty() ? TEXT("(None)") : *TargetAsset)))
						.ColorAndOpacity(FLinearColor(0.72f, 0.84f, 1.0f, 1.0f))
						.Font(FAppStyle::Get().GetFontStyle("SmallFont"))
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					BuildAgentBadge(ReceiverAgent)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 8.0f, 0.0f, 4.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Task Begin")))
				.Font(FAppStyle::Get().GetFontStyle("SmallFontBold"))
				.ColorAndOpacity(FLinearColor(0.9f, 0.78f, 0.45f, 1.0f))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(ItemsHost, SVerticalBox)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 10.0f, 0.0f, 0.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SButton)
					.Text(FText::FromString(GetLocString(TEXT("Accept"), TEXT("同意"))))
					.ButtonStyle(FAppStyle::Get(), "PrimaryButton")
					// .OnClicked ... Accept task trigger
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(FText::FromString(GetLocString(TEXT("Reject"), TEXT("拒绝"))))
					.ButtonStyle(FAppStyle::Get(), "Button")
					// .OnClicked ... Reject task trigger
				]
			]
		]
	];

	if (ItemsHost.IsValid())
	{
		for (int32 ItemIndex = 0; ItemIndex < Items.Num(); ++ItemIndex)
		{
			TSharedPtr<SEditableTextBox> FeedbackInput;
			TSharedPtr<STextBlock> FeedbackText;
			TSharedPtr<SButton> AnnotateButton;

			ItemsHost->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
				.Padding(FMargin(8.0f, 6.0f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(FString::Printf(TEXT("%d. %s"), ItemIndex + 1, *FormatTaskItemSingleLine(Items[ItemIndex]))))
							.AutoWrapText(false)
							.ColorAndOpacity(FLinearColor::White)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(6.0f, 0.0f, 0.0f, 0.0f)
						[
							SAssignNew(AnnotateButton, SButton)
							.ButtonStyle(FAppStyle::Get(), "FlatButton")
							.Text(FText::FromString(GetLocString(TEXT("Annotate"), TEXT("批注"))))
							// Expand logic
						]
					]
				]
			];
		}
	}
}

FString STaskBeginNode::FormatTaskItemSingleLine(const FString& InText)
{
	const FString Flattened = InText.Replace(TEXT("\n"), TEXT(" ")).Replace(TEXT("\t"), TEXT(" "));
	FString Out;
	Out.Reserve(Flattened.Len());
	bool bPrevSpace = false;
	for (TCHAR Ch : Flattened)
	{
		const bool bIsSpace = FChar::IsWhitespace(Ch);
		if (bIsSpace)
		{
			if (!bPrevSpace) Out.AppendChar(TEXT(' '));
			bPrevSpace = true;
			continue;
		}
		Out.AppendChar(Ch);
		bPrevSpace = false;
	}
	return Out.TrimStartAndEnd();
}

// Removed member func BuildAgentBadge
