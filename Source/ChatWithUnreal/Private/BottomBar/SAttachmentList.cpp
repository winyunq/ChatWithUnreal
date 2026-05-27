// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#include "SAttachmentList.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/AppStyle.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "FabServer/ChatSystem/UmgMcpActiveMessageSubsystem.h"
#include "Engine/Texture2D.h"
#include "Editor.h"

void SAttachmentList::Construct(const FArguments& InArgs)
{
	OnAttachmentAddedEvent = InArgs._OnAttachmentAdded;
	OnAttachmentRemovedEvent = InArgs._OnAttachmentRemoved;

	ChildSlot
	[
		SAssignNew(ListContainer, SHorizontalBox)
	];

	if (GEditor)
	{
		if (auto* Subsystem = GEditor->GetEditorSubsystem<UActiveMessageSubsystem>())
		{
			Subsystem->RegisterAttachmentList(SharedThis(this));
		}
	}
}

TArray<FString> SAttachmentList::GetBase64Images() const
{
	TArray<FString> Res;
	for (const auto& Item : AttachedImages)
	{
		Res.Add(Item.Base64Data);
	}
	return Res;
}

void SAttachmentList::ClearAttachments()
{
	for (auto& Item : AttachedImages)
	{
		if (Item.SlateBrush.IsValid())
		{
			Item.SlateBrush->ReleaseResource();
		}
	}
	AttachedImages.Empty();

	if (GEditor)
	{
		if (auto* Subsystem = GEditor->GetEditorSubsystem<UActiveMessageSubsystem>())
		{
			Subsystem->ClearTextureCache();
		}
	}

	RefreshList();
}

void SAttachmentList::AddAttachment(const FString& InBase64, const FString& InImageId)
{
	FString ImageId = InImageId;
	if (ImageId.IsEmpty())
	{
		ImageIndexCounter++;
		ImageId = FString::Printf(TEXT("image%d"), ImageIndexCounter);
	}

	UTexture2D* Texture = nullptr;
	if (GEditor)
	{
		if (auto* Subsystem = GEditor->GetEditorSubsystem<UActiveMessageSubsystem>())
		{
			FString CacheKey = FString::Printf(TEXT("Attachment_%s"), *ImageId);
			Texture = Subsystem->GetOrCreateDynamicTexture(InBase64, CacheKey);
		}
	}

	TSharedPtr<FSlateDynamicImageBrush> Brush = nullptr;
	if (Texture)
	{
		float W = Texture->GetSizeX();
		float H = Texture->GetSizeY();

		// 自然对数缩放核心算法：
		// Scale = 1.0 + ln(MinEdge / BaseSize)
		float BaseSize = 64.0f;
		float MinEdge = FMath::Min(W, H);
		float Scale = 1.0f;
		if (MinEdge > BaseSize)
		{
			Scale = 1.0f + FMath::Loge(MinEdge / BaseSize);
		}
		float TargetMinEdge = BaseSize * Scale;
		TargetMinEdge = FMath::Clamp(TargetMinEdge, BaseSize, 120.0f); // 缩略图最小边约束在 64-120px 之间

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

		Brush = MakeShareable(new FSlateDynamicImageBrush(Texture, TargetSize, NAME_None));
	}

	AttachedImages.Add({ ImageId, InBase64, Brush });
	RefreshList();

	OnAttachmentAddedEvent.ExecuteIfBound(ImageId);
}

void SAttachmentList::RemoveAttachmentById(const FString& InImageId)
{
	for (int32 i = 0; i < AttachedImages.Num(); ++i)
	{
		if (AttachedImages[i].ImageId == InImageId)
		{
			if (AttachedImages[i].SlateBrush.IsValid())
			{
				AttachedImages[i].SlateBrush->ReleaseResource();
			}
			OnAttachmentRemovedEvent.ExecuteIfBound(InImageId);
			AttachedImages.RemoveAt(i);
			break;
		}
	}
	RefreshList();
}

void SAttachmentList::RefreshList()
{
	ListContainer->ClearChildren();

	for (int32 i = 0; i < AttachedImages.Num(); ++i)
	{
		const auto& Item = AttachedImages[i];

		TSharedRef<SWidget> ImageWidget = SNew(SImage)
			.Image(FAppStyle::Get().GetBrush("Icons.Image")); // 默认占位

		FVector2D BoxSize(64.0f, 64.0f);

		if (Item.SlateBrush.IsValid())
		{
			ImageWidget = SNew(SImage)
				.Image(Item.SlateBrush.Get());
			BoxSize = Item.SlateBrush->ImageSize;
		}

		ListContainer->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Bottom)
		.Padding(6.0f, 4.0f)
		[
			SNew(SBox)
			.WidthOverride(BoxSize.X)
			.HeightOverride(BoxSize.Y)
			[
				SNew(SOverlay)
				
				// 1. 缩略图片本体 (带圆角边框包围)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("DetailsView.CategoryTop"))
					.Padding(0.0f)
					.BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
					[
						ImageWidget
					]
				]

				// 2. 底部半透明遮罩与图片编号显示 (炫酷的 [imageN] 标签)
				+ SOverlay::Slot()
				.VAlign(VAlign_Bottom)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f)) // 60% 科技黑半透明
					.Padding(FMargin(2.0f, 1.0f))
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Item.ImageId))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
						.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.95f, 1.0f))
					]
				]

				// 3. 右上角红色联动删除按钮
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
					.ContentPadding(0.0f)
					.OnClicked_Lambda([this, Item]() {
						// 本地项删除，自动触发 OnAttachmentRemovedEvent 委托通知
						this->RemoveAttachmentById(Item.ImageId);
						return FReply::Handled();
					})
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("Menu.Background"))
						.BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f, 0.8f))
						.Padding(FMargin(5.0f, 3.0f, 5.0f, 4.0f))
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("×")))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
							.ColorAndOpacity(FLinearColor(0.75f, 0.75f, 0.75f, 1.0f))
						]
					]
				]
			]
		];
	}

	SetVisibility(AttachedImages.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed);
}
