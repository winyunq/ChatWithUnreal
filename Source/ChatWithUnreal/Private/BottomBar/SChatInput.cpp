// Copyright (c) 2025-2026 Winyunq. All rights reserved.
#include "SChatInput.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/SBoxPanel.h"

#include "FabServer/FabWindowsClipboard.h"
#include "ImageUtils.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "FabServer/ChatSystem/UmgMcpActiveMessageSubsystem.h"
#include "SAttachmentList.h"
#include "Input/DragAndDrop.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"








#include "Editor.h"

namespace
{
	FText GetLocText(const FString& English, const FString& Chinese)
	{
		const FString Culture = FInternationalization::Get().GetCurrentCulture()->GetName();
		return FText::FromString(Culture.StartsWith(TEXT("zh")) ? Chinese : English);
	}
}

void SChatInput::Construct(const FArguments& InArgs)
{
	bIsUpdatingText = false;
	LastText = TEXT("");

	ChildSlot
	[
		SAssignNew(InputTextBox, SMultiLineEditableTextBox)
		.AutoWrapText(true)
		.HintText(GetLocText(TEXT("Type a message... (Ctrl+Enter to Send)"), TEXT("输入消息... (Ctrl+Enter 发送)")))
		.OnKeyDownHandler(this, &SChatInput::OnInputKeyDown)
		.OnTextChanged(this, &SChatInput::HandleTextChanged)
	];

	if (GEditor)
	{
		if (auto* Subsystem = GEditor->GetEditorSubsystem<UActiveMessageSubsystem>())
		{
			Subsystem->RegisterChatInput(SharedThis(this));
		}
	}
}

FText SChatInput::GetText() const
{
	if (!InputTextBox.IsValid()) return FText::GetEmpty();

	FString RawText = InputTextBox->GetText().ToString();
	FString ResultText;
	int32 ImageCounter = 0;

	for (int32 i = 0; i < RawText.Len(); ++i)
	{
		if (RawText[i] == 0x25C6) // Technology Diamond Character ◆
		{
			ImageCounter++;
			ResultText += FString::Printf(TEXT("<WinyunqImageBegin>image%d<WinyunqImageEnd>"), ImageCounter);
		}
		else
		{
			ResultText.AppendChar(RawText[i]);
		}
	}
	return FText::FromString(ResultText);
}

void SChatInput::SetText(const FText& InText)
{
	if (InputTextBox.IsValid())
	{
		FString RawText = InText.ToString();
		int32 SearchPos = 0;
		while ((SearchPos = RawText.Find(TEXT("<WinyunqImageBegin>"), ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchPos)) != INDEX_NONE)
		{
			int32 EndTagIdx = RawText.Find(TEXT("<WinyunqImageEnd>"), ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchPos + 19);
			if (EndTagIdx != INDEX_NONE)
			{
				RawText.RemoveAt(SearchPos, (EndTagIdx + 17) - SearchPos);
				RawText.InsertAt(SearchPos, TEXT("\u25C6"));
				SearchPos += 1;
			}
			else
			{
				SearchPos += 19;
			}
		}
		
		bIsUpdatingText = true;
		InputTextBox->SetText(FText::FromString(RawText));
		LastText = RawText;
		bIsUpdatingText = false;
	}
}

void SChatInput::ClearText()
{
	if (InputTextBox.IsValid())
	{
		bIsUpdatingText = true;
		InputTextBox->SetText(FText::GetEmpty());
		LastText = TEXT("");
		bIsUpdatingText = false;
	}
}

FReply SChatInput::OnInputKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// 发送快捷键：Enter（当未按Shift时）或Ctrl+Enter
	bool bIsEnter = InKeyEvent.GetKey() == EKeys::Enter;
	bool bHasCtrl = InKeyEvent.IsControlDown();
	bool bHasShift = InKeyEvent.IsShiftDown();

	if (bIsEnter && (!bHasShift || bHasCtrl))
	{
		if (GEditor)
		{
			if (auto* Subsystem = GEditor->GetEditorSubsystem<UActiveMessageSubsystem>())
			{
				Subsystem->ExecuteSendMessage();
			}
		}
		return FReply::Handled();
	}

#if PLATFORM_WINDOWS
	// 粘贴图片快捷键：Ctrl+V
	if (bHasCtrl && InKeyEvent.GetKey() == EKeys::V)
	{
		int32 Width, Height;
		TArray<uint8> PendingImageData;
		if (FFabWindowsClipboard::GetBitmapFromClipboard(PendingImageData, Width, Height))
		{
			if (GEditor)
			{
				if (auto* Subsystem = GEditor->GetEditorSubsystem<UActiveMessageSubsystem>())
				{
					FString Base64Str = FBase64::Encode(PendingImageData);
					if (auto AttachmentList = Subsystem->GetRegisteredAttachmentList())
					{
						AttachmentList->AddAttachment(Base64Str);
					}
				}
			}
			return FReply::Handled(); 
		}
	}
#endif

	return FReply::Unhandled();
}










FReply SChatInput::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FExternalDragOperation> ExternalDrag = DragDropEvent.GetOperationAs<FExternalDragOperation>();
	if (ExternalDrag.IsValid() && ExternalDrag->HasFiles())
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SChatInput::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FExternalDragOperation> ExternalDrag = DragDropEvent.GetOperationAs<FExternalDragOperation>();
	if (ExternalDrag.IsValid() && ExternalDrag->HasFiles())
	{
		const TArray<FString>& Files = ExternalDrag->GetFiles();
		bool bLoadedAny = false;
		for (const FString& FilePath : Files)
		{
			FString Ext = FPaths::GetExtension(FilePath).ToLower();
			if (Ext == TEXT("png") || Ext == TEXT("jpg") || Ext == TEXT("jpeg") || Ext == TEXT("bmp") || Ext == TEXT("wav") || Ext == TEXT("mp3"))
			{
				TArray<uint8> FileData;
				if (FFileHelper::LoadFileToArray(FileData, *FilePath))
				{
					FString Base64Str = FBase64::Encode(FileData);
					if (GEditor)
					{
						if (auto* Subsystem = GEditor->GetEditorSubsystem<UActiveMessageSubsystem>())
						{
							if (auto AttachmentList = Subsystem->GetRegisteredAttachmentList())
							{
								AttachmentList->AddAttachment(Base64Str);
								bLoadedAny = true;
							}
						}
					}
				}
			}
		}
		if (bLoadedAny)
		{
			return FReply::Handled();
		}
	}
	return FReply::Unhandled();
}


void SChatInput::RemoveImageTag(const FString& InImageId)
{
	if (!InputTextBox.IsValid()) return;
	
	if (GEditor)
	{
		if (auto* Subsystem = GEditor->GetEditorSubsystem<UActiveMessageSubsystem>())
		{
			if (auto AttachmentList = Subsystem->GetRegisteredAttachmentList())
			{
				const TArray<FUmgMcpAttachmentItem>& Items = AttachmentList->GetAttachmentItems();
				int32 TargetIndex = -1;
				for (int32 i = 0; i < Items.Num(); ++i)
				{
					if (Items[i].ImageId == InImageId)
					{
						TargetIndex = i + 1; // 物理位置
						break;
					}
				}
				
				if (TargetIndex > 0)
				{
					FString CurrentText = InputTextBox->GetText().ToString();
					int32 FoundCount = 0;
					for (int32 i = 0; i < CurrentText.Len(); ++i)
					{
						if (CurrentText[i] == 0x25C6) // ◆
						{
							FoundCount++;
							if (FoundCount == TargetIndex)
							{
								CurrentText.RemoveAt(i, 1);
								break;
							}
						}
					}
					
					bIsUpdatingText = true;
					InputTextBox->SetText(FText::FromString(CurrentText));
					LastText = CurrentText;
					bIsUpdatingText = false;
				}
			}
		}
	}
}

void SChatInput::InsertImageTag(const FString& InImageId)
{
	if (InputTextBox.IsValid())
	{
		bIsUpdatingText = true;
		InputTextBox->InsertTextAtCursor(FText::FromString(TEXT("\u25C6")));
		LastText = InputTextBox->GetText().ToString();
		bIsUpdatingText = false;
	}
}

void SChatInput::HandleTextChanged(const FText& NewText)
{
	if (bIsUpdatingText) return;

	FString NewTextStr = NewText.ToString();

	if (GEditor)
	{
		if (auto* Subsystem = GEditor->GetEditorSubsystem<UActiveMessageSubsystem>())
		{
			if (auto AttachmentList = Subsystem->GetRegisteredAttachmentList())
			{
				// 1. 分别统计旧文本和新文本中的 ◆ 数量及物理索引位置
				TArray<int32> OldTokenIndices;
				TArray<int32> NewTokenIndices;

				for (int32 i = 0; i < LastText.Len(); ++i)
				{
					if (LastText[i] == 0x25C6) // ◆
					{
						OldTokenIndices.Add(i);
					}
				}

				for (int32 i = 0; i < NewTextStr.Len(); ++i)
				{
					if (NewTextStr[i] == 0x25C6) // ◆
					{
						NewTokenIndices.Add(i);
					}
				}

				const TArray<FUmgMcpAttachmentItem>& Items = AttachmentList->GetAttachmentItems();

				// 2. 如果占位符数量减少了，代表用户通过打字编辑（如 Backspace 等）物理删除了占位符！
				if (NewTokenIndices.Num() < OldTokenIndices.Num() && Items.Num() > 0)
				{
					if (NewTokenIndices.Num() == 0)
					{
						bIsUpdatingText = true;
						AttachmentList->ClearAttachments();
						bIsUpdatingText = false;
					}
					else
					{
						// 用前缀上下文比对算法，精准找出是哪个占位符被干掉了
						int32 ErasedIndex = -1;
						for (int32 i = 0; i < NewTokenIndices.Num(); ++i)
						{
							FString NewPrefix = NewTextStr.Left(NewTokenIndices[i]).Replace(TEXT("◆"), TEXT(""));
							FString OldPrefix = LastText.Left(OldTokenIndices[i]).Replace(TEXT("◆"), TEXT(""));

							if (NewPrefix != OldPrefix)
							{
								ErasedIndex = i;
								break;
							}
						}

						if (ErasedIndex == -1)
						{
							ErasedIndex = OldTokenIndices.Num() - 1;
						}

						// 双向联动：反向将对应的图片从附录中剔除销毁，实现完美的闭环状态一致性
						if (ErasedIndex >= 0 && ErasedIndex < Items.Num())
						{
							bIsUpdatingText = true;
							AttachmentList->RemoveAttachmentById(Items[ErasedIndex].ImageId);
							bIsUpdatingText = false;
						}
					}
				}
			}
		}
	}

	LastText = NewTextStr;
}

#undef LOCTEXT_NAMESPACE