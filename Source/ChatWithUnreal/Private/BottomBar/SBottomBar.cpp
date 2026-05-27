// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#include "SBottomBar.h"

#include "SAbilitiesSelector.h"
#include "SChatInput.h"
#include "SChatSendButton.h"
#include "SInteractionModeSelector.h"
#include "SToolModeSelector.h"
#include "SAttachmentList.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformApplicationMisc.h"
#include "SQuotaBar.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSpacer.h"

void SBottomBar::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		
				// 0. 附件列表
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 5.0f)
		[
			SAssignNew(AttachmentList, SAttachmentList)
			.OnAttachmentAdded_Lambda([this](const FString& ImageId) {
				if (ChatInput.IsValid())
				{
					ChatInput->InsertImageTag(ImageId);
				}
			})
			.OnAttachmentRemoved_Lambda([this](const FString& ImageId) {
				if (ChatInput.IsValid())
				{
					ChatInput->RemoveImageTag(ImageId);
				}
			})
		]

		// 1. 输入框
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(ChatInput, SChatInput)
		]

		// 2. 控制工具条 (原子控件拼装)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 5.0f, 0.0f, 0.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(InteractionModeSelector, SInteractionModeSelector)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f, 0.0f, 0.0f, 0.0f)
			[
				SAssignNew(AbilitiesSelector, SAbilitiesSelector)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f, 0.0f, 0.0f, 0.0f)
			[
				SAssignNew(ToolModeSelector, SToolModeSelector)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SSpacer)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f, 0.0f)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
				.ToolTipText(FText::FromString(TEXT("Add Image Attachment")))
				.OnClicked(this, &SBottomBar::OnAddAttachmentClicked)
				[
					SNew(SImage)
					.Image(FAppStyle::Get().GetBrush("Icons.Plus"))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(SendButton, SChatSendButton)
			]
		]

		// 3. 配额进度条
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 5.0f, 0.0f, 0.0f)
		[
			SAssignNew(QuotaBar, SQuotaBar)
			.Percent(0.5f)
			.Visibility(EVisibility::Collapsed) 
		]
	];
}

void SBottomBar::OnInteractionModeChanged(const FString& NewMode)
{
}

void SBottomBar::OnChatInputSendRequested()
{
}

void SBottomBar::OnChatInputPasteImage(const TArray<uint8>& ImageData, int32 Width, int32 Height)
{
}

void SBottomBar::OnToolModeChanged(const FString& NewTool)
{
}
FReply SBottomBar::OnAddAttachmentClicked()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform) return FReply::Handled();

	TArray<FString> OutFiles;
	void* ParentWindowHandle = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();

	if (DesktopPlatform->OpenFileDialog(
		ParentWindowHandle,
		TEXT("Select Image Attachment"),
		TEXT(""),
		TEXT(""),
		TEXT("Image Files (*.png;*.jpg;*.jpeg)|*.png;*.jpg;*.jpeg"),
		EFileDialogFlags::None,
		OutFiles
	))
	{
		for (const FString& FilePath : OutFiles)
		{
			TArray<uint8> FileData;
			if (FFileHelper::LoadFileToArray(FileData, *FilePath))
			{
				FString Base64Str = FBase64::Encode(FileData);
				if (AttachmentList.IsValid())
				{
					AttachmentList->AddAttachment(Base64Str);
				}
			}
		}
	}

	return FReply::Handled();
}
