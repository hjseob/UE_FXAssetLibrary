// Fill out your copyright notice in the Description page of Project Settings.

#include "SFXAssetRegistPanel.h"
#include "SlateOptMacros.h"

// Core includes first
#include "CoreMinimal.h"


// Modules (before Asset Registry to ensure proper namespace)
#include "Modules/ModuleManager.h"
#include "AssetRegistry/AssetData.h"


// Asset Registry (after modules)
#include "AssetRegistry/IAssetRegistry.h"
#include "AssetRegistry/AssetRegistryModule.h"


// Slate Widgets
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"





// Content Browser
#include "ContentBrowserModule.h"

// FX Library
#include "FXLibrarySettings.h"
#include "FXAssetOrganizer.h"
#include "FXAssetMover.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SFXAssetRegistPanel::Construct(const FArguments& InArgs)
{
	SelectedAssets = InArgs._SelectedAssets;
	
	// 기본값 설정
	RootPath = TEXT("/Game/FXLib/");
	
	// 첫 번째 선택된 에셋의 이름을 기본값으로 사용
	if (SelectedAssets.Num() > 0)
	{
		AssetName = SelectedAssets[0].AssetName.ToString();
		CategoryName = TEXT("Default");
	}

	// 카테고리 목록 로드
	LoadCategoryOptions();

	ChildSlot
	[
		SNew(SBorder)
		.Padding(20)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)

			// 제목
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 20)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Register FX Asset")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
			]

			// Root FX Path
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 5)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Root FX Path")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SEditableTextBox)
					.Text(FText::FromString(RootPath))
					.OnTextChanged(this, &SFXAssetRegistPanel::OnRootPathChanged)
					.HintText(FText::FromString(TEXT("/Game/Test/")))
				]
			]

			// Asset Name
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 5)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Asset Name")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SEditableTextBox)
					.Text(FText::FromString(AssetName))
					.OnTextChanged(this, &SFXAssetRegistPanel::OnAssetNameChanged)
					.HintText(FText::FromString(TEXT("Enter asset name")))
				]
			]

			// Category Name (ComboBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 5)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Category Name")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SAssignNew(CategoryComboBox, SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&CategoryOptions)
						.OnGenerateWidget(this, &SFXAssetRegistPanel::MakeCategoryWidget)
						.OnSelectionChanged(this, &SFXAssetRegistPanel::OnCategorySelectionChanged)
						.Content()
						[
							SNew(STextBlock)
							.Text(this, &SFXAssetRegistPanel::GetCategoryComboBoxContent)
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(5, 0, 0, 0)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("+")))
						.ToolTipText(FText::FromString(TEXT("Add new category")))
						.OnClicked(this, &SFXAssetRegistPanel::OnAddCategoryClicked)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
					]
				]
			]

			// Hashtags (주석 처리)
			/*
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 20)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 5)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Hashtags (comma separated)")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SEditableTextBox)
					.Text(FText::FromString(Hashtags))
					.OnTextChanged(this, &SFXAssetRegistPanel::OnHashtagsChanged)
					.HintText(FText::FromString(TEXT("fire, explosion, vfx")))
				]
			]
			*/

			// 버튼들
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5, 0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Cancel")))
					.OnClicked(this, &SFXAssetRegistPanel::OnCancelClicked)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5, 0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Register")))
					.ButtonStyle(FAppStyle::Get(), "PrimaryButton")
					.OnClicked(this, &SFXAssetRegistPanel::OnRegisterClicked)
				]
			]
		]
	];
}

FReply SFXAssetRegistPanel::OnRegisterClicked()
{
	// 유효성 검사
	if (RootPath.IsEmpty() || AssetName.IsEmpty() || CategoryName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Please fill in all required fields"));
		return FReply::Handled();
	}

	// Settings 가져오기
	UFXLibrarySettings* Settings = GetMutableDefault<UFXLibrarySettings>();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get FX Library Settings"));
		return FReply::Handled();
	}

	// 카테고리 찾기 또는 생성
	FFXCategoryData* Category = Settings->FindCategory(*CategoryName);
	if (!Category)
	{
		Category = Settings->AddCategory(*CategoryName);
		UE_LOG(LogTemp, Log, TEXT("Created new category: %s"), *CategoryName);
	}

	// 선택된 에셋들 중 나이아가라 시스템만 복사 및 등록
	int32 AddedCount = 0;
	for (const FAssetData& AssetData : SelectedAssets)
	{
		if (AssetData.AssetClassPath.GetAssetName() == "NiagaraSystem"
			|| AssetData.AssetClassPath.ToString().Contains("NiagaraSystem"))
		{
			// 원본 에셋 경로
			FSoftObjectPath SourceAssetPath = AssetData.ToSoftObjectPath();
			
			// 대상 폴더 경로 생성 (RootPath/Particles/CategoryName)
			FString DestinationFolder = FXAssetOrganizer::GetFolderPathForAssetType(
				RootPath, CategoryName, TEXT("NiagaraSystem"));
			
			// 폴더 생성
			if (!FXAssetOrganizer::CreateFolderPath(DestinationFolder))
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to create folder: %s"), *DestinationFolder);
				continue;
			}
			
			// 에셋 복사 (새 이름으로)
			FSoftObjectPath CopiedAssetPath = FXAssetMover::CopyAssetWithNewName(
				SourceAssetPath,
				DestinationFolder,
				AssetName
			);
			
			if (CopiedAssetPath.IsValid())
			{
				// 복사된 에셋을 카테고리에 추가
				if (!Category->Assets.Contains(CopiedAssetPath))
				{
					Category->Assets.Add(CopiedAssetPath);
					AddedCount++;
					UE_LOG(LogTemp, Log, TEXT("Copied and registered asset: %s -> %s"), 
						*SourceAssetPath.ToString(), *CopiedAssetPath.ToString());
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to copy asset: %s"), *SourceAssetPath.ToString());
			}
		}
	}

	// 설정 저장
	Settings->SaveConfig();
	Settings->TryUpdateDefaultConfigFile();

	UE_LOG(LogTemp, Log, TEXT("Registered %d assets to category: %s (Root: %s)"), 
		AddedCount, *CategoryName, *RootPath);

	// 창 닫기
	if (TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared()))
	{
		ParentWindow->RequestDestroyWindow();
	}

	return FReply::Handled();
}

FReply SFXAssetRegistPanel::OnCancelClicked()
{
	// 창 닫기
	if (TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared()))
	{
		ParentWindow->RequestDestroyWindow();
	}
	return FReply::Handled();
}

void SFXAssetRegistPanel::OnRootPathChanged(const FText& NewText)
{
	RootPath = NewText.ToString();
}

void SFXAssetRegistPanel::OnAssetNameChanged(const FText& NewText)
{
	AssetName = NewText.ToString();
}

void SFXAssetRegistPanel::OnCategorySelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection.IsValid())
	{
		CategoryName = *NewSelection;
		SelectedCategoryOption = NewSelection;
	}
}

FReply SFXAssetRegistPanel::OnAddCategoryClicked()
{
	// 간단한 입력 다이얼로그 생성
	FText InputText;
	FText Title = FText::FromString(TEXT("New Category"));
	FText Message = FText::FromString(TEXT("Enter category name:"));
	FString DefaultValue = TEXT("NewCategory");
	
	// 입력 다이얼로그 창 생성
	TSharedRef<SWindow> InputWindow = SNew(SWindow)
		.Title(Title)
		.SizingRule(ESizingRule::UserSized)
		.ClientSize(FVector2D(400, 150))
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	TSharedPtr<SEditableTextBox> InputTextBox;
	
	TSharedRef<SWidget> InputContent = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20, 20, 20, 10)
		[
			SNew(STextBlock)
			.Text(Message)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20, 0, 20, 20)
		[
			SAssignNew(InputTextBox, SEditableTextBox)
			.Text(FText::FromString(DefaultValue))
			.SelectAllTextWhenFocused(true)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.Padding(20, 0, 20, 20)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5, 0)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Cancel")))
				.OnClicked_Lambda([InputWindow]()
				{
					InputWindow->RequestDestroyWindow();
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5, 0)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Add")))
				.ButtonStyle(FAppStyle::Get(), "PrimaryButton")
				.OnClicked_Lambda([this, InputWindow, InputTextBox]()
				{
					FString NewCategoryName = InputTextBox->GetText().ToString().TrimStartAndEnd();
					if (!NewCategoryName.IsEmpty())
					{
						UFXLibrarySettings* Settings = GetMutableDefault<UFXLibrarySettings>();
						if (Settings)
						{
							// 카테고리 추가
							Settings->AddCategory(*NewCategoryName);
							Settings->SaveConfig();
							
							// 목록 새로고침
							LoadCategoryOptions();
							
							// 새로 추가된 카테고리 선택
							if (CategoryComboBox.IsValid())
							{
								for (const TSharedPtr<FString>& Option : CategoryOptions)
								{
									if (Option.IsValid() && *Option == NewCategoryName)
									{
										CategoryComboBox->SetSelectedItem(Option);
										CategoryName = NewCategoryName;
										break;
									}
								}
							}
						}
					}
					InputWindow->RequestDestroyWindow();
					return FReply::Handled();
				})
			]
		];

	InputWindow->SetContent(InputContent);
	FSlateApplication::Get().AddModalWindow(InputWindow, nullptr);
	
	// 포커스를 입력 박스에 설정
	if (InputTextBox.IsValid())
	{
		FSlateApplication::Get().SetKeyboardFocus(InputTextBox, EFocusCause::SetDirectly);
	}
	
	return FReply::Handled();
}

void SFXAssetRegistPanel::LoadCategoryOptions()
{
	CategoryOptions.Empty();
	
	UFXLibrarySettings* Settings = GetMutableDefault<UFXLibrarySettings>();
	if (Settings)
	{
		for (const FFXCategoryData& Category : Settings->Categories)
		{
			CategoryOptions.Add(MakeShareable(new FString(Category.CategoryName.ToString())));
		}
	}
	
	// 기본값이 없으면 "Default" 추가
	if (CategoryOptions.Num() == 0)
	{
		CategoryOptions.Add(MakeShareable(new FString(TEXT("Default"))));
	}
	
	// 현재 선택된 카테고리가 있으면 선택 유지
	if (SelectedCategoryOption.IsValid())
	{
		FString SelectedName = *SelectedCategoryOption;
		for (const TSharedPtr<FString>& Option : CategoryOptions)
		{
			if (Option.IsValid() && *Option == SelectedName)
			{
				CategoryComboBox->SetSelectedItem(Option);
				return;
			}
		}
	}
	
	// 기본값 선택 (CategoryComboBox가 유효한 경우에만)
	if (CategoryOptions.Num() > 0 && CategoryComboBox.IsValid())
	{
		CategoryComboBox->SetSelectedItem(CategoryOptions[0]);
		CategoryName = *CategoryOptions[0];
		SelectedCategoryOption = CategoryOptions[0];
	}
}

TSharedRef<SWidget> SFXAssetRegistPanel::MakeCategoryWidget(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock).Text(FText::FromString(*InOption));
}

FText SFXAssetRegistPanel::GetCategoryComboBoxContent() const
{
	if (SelectedCategoryOption.IsValid())
	{
		return FText::FromString(*SelectedCategoryOption);
	}
	return FText::FromString(CategoryName.IsEmpty() ? TEXT("Select Category") : CategoryName);
}

void SFXAssetRegistPanel::OnHashtagsChanged(const FText& NewText)
{
	Hashtags = NewText.ToString();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
