// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#include "Messages/SUserMessageWidget.h"
#include "SUserAvatar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "FabServer/ChatSystem/UmgMcpActiveMessageSubsystem.h"
#include "Engine/Texture2D.h"
#include "Editor.h"

#include "FabServer/Authentication/UmgMcpAuthenticationSubsystem.h"

namespace
{
	static FString CleanImageTags(const FString& InText)
	{
		FString Result = InText;
		int32 StartIdx = 0;
		while ((StartIdx = Result.Find(TEXT("<WinyunqImageBegin>"))) != INDEX_NONE)
		{
			int32 EndIdx = Result.Find(TEXT("<WinyunqImageEnd>"), ESearchCase::CaseSensitive, ESearchDir::FromStart, StartIdx);
			if (EndIdx != INDEX_NONE)
			{
				Result.RemoveAt(StartIdx, (EndIdx + 17) - StartIdx);
			}
			else
			{
				Result.RemoveAt(StartIdx, 19);
			}
		}
		return Result;
	}
}

void SUserMessageWidget::Construct(const FArguments& InArgs)
{
	MessageText = InArgs._MessageText;
	Base64Images = InArgs._Base64Images;

	FString DisplayName = TEXT("User");
	if (GEditor)
	{
		if (UAuthenticationSubsystem* AuthSys = GEditor->GetEditorSubsystem<UAuthenticationSubsystem>())
		{
			if (AuthSys->IsAnyAccountLoggedIn())
			{
				DisplayName = AuthSys->GetActiveUserDisplayName();
			}
			else
			{
				DisplayName = AuthSys->GetLocalUserDisplayName();
			}
		}
	}

	TSharedPtr<SVerticalBox> BubbleContentBox = SNew(SVerticalBox);

	// ---- 自然对数解析图文混排的核心算法 ----
	FString ParsingText = MessageText;
	int32 CurrentPos = 0;

	while (CurrentPos < ParsingText.Len())
	{
		int32 BeginIdx = ParsingText.Find(TEXT("<WinyunqImageBegin>"), ESearchCase::CaseSensitive, ESearchDir::FromStart, CurrentPos);
		if (BeginIdx == INDEX_NONE)
		{
			FString NormalPart = CleanImageTags(ParsingText.RightChop(CurrentPos)).TrimStartAndEnd();
			if (!NormalPart.IsEmpty())
			{
				BubbleContentBox->AddSlot()
				.AutoHeight()
				.Padding(0.0f, 2.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(NormalPart))
					.AutoWrapText(true)
					.ColorAndOpacity(FLinearColor::White)
					.Font(FAppStyle::Get().GetFontStyle("NormalFont"))
				];
			}
			break;
		}

		if (BeginIdx > CurrentPos)
		{
			FString NormalPart = CleanImageTags(ParsingText.Mid(CurrentPos, BeginIdx - CurrentPos)).TrimStartAndEnd();
			if (!NormalPart.IsEmpty())
			{
				BubbleContentBox->AddSlot()
				.AutoHeight()
				.Padding(0.0f, 2.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(NormalPart))
					.AutoWrapText(true)
					.ColorAndOpacity(FLinearColor::White)
					.Font(FAppStyle::Get().GetFontStyle("NormalFont"))
				];
			}
		}

		int32 ContentStartIdx = BeginIdx + 19; // strlen("<WinyunqImageBegin>") = 19
		int32 EndIdx = ParsingText.Find(TEXT("<WinyunqImageEnd>"), ESearchCase::CaseSensitive, ESearchDir::FromStart, ContentStartIdx);
		if (EndIdx == INDEX_NONE)
		{
			// 破损标签安全处理：跳过起始标记，避免泄露
			CurrentPos = BeginIdx + 19;
			continue;
		}

		FString ImageId = ParsingText.Mid(ContentStartIdx, EndIdx - ContentStartIdx);
		int32 ImageIndex = -1;
		if (ImageId.StartsWith(TEXT("image")))
		{
			ImageIndex = FCString::Atoi(*ImageId.RightChop(5)) - 1;
		}

		if (ImageIndex >= 0 && ImageIndex < Base64Images.Num())
		{
			const FString& Base64Str = Base64Images[ImageIndex];
			UTexture2D* Texture = nullptr;
			if (GEditor)
			{
				if (auto* Subsystem = GEditor->GetEditorSubsystem<UActiveMessageSubsystem>())
				{
					FString CacheKey = FString::Printf(TEXT("Bubble_%p_%s"), this, *ImageId);
					Texture = Subsystem->GetOrCreateDynamicTexture(Base64Str, CacheKey);
				}
			}

			if (Texture)
			{
				float W = Texture->GetSizeX();
				float H = Texture->GetSizeY();

				// 对数比例算法计算最舒服的气泡图尺寸
				float BaseSize = 64.0f;
				float MinEdge = FMath::Min(W, H);
				float Scale = 1.0f;
				if (MinEdge > BaseSize)
				{
					Scale = 1.0f + FMath::Loge(MinEdge / BaseSize);
				}
				float TargetMinEdge = BaseSize * Scale;
				TargetMinEdge = FMath::Clamp(TargetMinEdge, BaseSize, 180.0f); // 气泡内最大边限定在 180px

				float AspectRatio = W / H;
				FVector2D TargetSize;
				if (W <= H)
				{
					TargetSize.X = TargetMinEdge;
					TargetSize.Y = TargetMinEdge / AspectRatio;
				}
				else
				{
					TargetSize.Y = TargetMinEdge;
					TargetSize.X = TargetMinEdge * AspectRatio;
				}

				TSharedPtr<FSlateDynamicImageBrush> DynamicBrush = MakeShareable(new FSlateDynamicImageBrush(Texture, TargetSize, NAME_None));
				AttachedSlateBrushes.Add(DynamicBrush);

				BubbleContentBox->AddSlot()
				.AutoHeight()
				.Padding(0.0f, 6.0f)
				[
					SNew(SBox)
					.WidthOverride(TargetSize.X)
					.HeightOverride(TargetSize.Y)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("DetailsView.CategoryTop"))
						.Padding(0.0f)
						.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.5f))
						[
							SNew(SImage)
							.Image(DynamicBrush.Get())
						]
					]
				];
			}
		}

		CurrentPos = EndIdx + 17; // strlen("<WinyunqImageEnd>") = 17
	}

	ChildSlot
	[
		SNew(SHorizontalBox)
		// 1. 左侧占位实现右对齐 (Push everything to the right)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SSpacer)
		]

		// 2. 消息内容列 (Bubble + Action Row)
		+ SHorizontalBox::Slot()
		.FillWidth(2.0f)
		.Padding(FMargin(60.0f, 0.0f, 12.0f, 0.0f))
		[
			SNew(SVerticalBox)
			// 顶部：名字
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(0.0f, 0.0f, 0.0f, 4.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(DisplayName))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.75f, 1.0f))
			]

			// 气泡
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SSpacer)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("DetailsView.CategoryTop"))
					.Padding(FMargin(12.0f, 8.0f))
					.BorderBackgroundColor(FLinearColor(0.2f, 0.4f, 0.8f, 1.0f))
					[
						BubbleContentBox.ToSharedRef()
					]
				]
			]

			// 操作条 (复制按钮)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ContentPadding(FMargin(4.0f, 2.0f))
				.OnClicked_Lambda([this]() {
					FString PlainText = CleanImageTags(MessageText).TrimStartAndEnd();
					FPlatformApplicationMisc::ClipboardCopy(*PlainText);
					return FReply::Handled();
				})
				.ToolTipText(FText::FromString(TEXT("Copy text")))
				[
					SNew(SImage)
					.Image(FAppStyle::Get().GetBrush("GenericCommands.Copy"))
					.DesiredSizeOverride(FVector2D(12, 12))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			]
		]

		// 3. 右侧头像
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Top)
		[
			SNew(SUserAvatar)
		]
	];
}

SUserMessageWidget::~SUserMessageWidget()
{
	for (auto& Brush : AttachedSlateBrushes)
	{
		if (Brush.IsValid())
		{
			Brush->ReleaseResource();
		}
	}
	AttachedSlateBrushes.Empty();
}
