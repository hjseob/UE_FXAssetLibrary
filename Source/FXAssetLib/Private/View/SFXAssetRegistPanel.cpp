// Fill out your copyright notice in the Description page of Project Settings.

#include "View/SFXAssetRegistPanel.h"
#include "SlateOptMacros.h"

// Controller & Model
#include "Controller/SFXAssetRegistPanelController.h"
#include "Model/FXLibraryModel.h"

// Utils
#include "Utils/FXAssetOrganizer.h"
#include "Utils/FXAssetMover.h"

// Core
#include "Core/FXAssetLibConstants.h"

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

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SFXAssetRegistPanel::Construct(const FArguments& InArgs)
{
	SelectedAssets = InArgs._SelectedAssets;
	
	//// Controller 및 Model 생성
	//TSharedPtr<FFXLibraryModel> Model = MakeShared<FFXLibraryModel>();
	//Controller = MakeShared<SFXAssetRegistPanelController>();
	//Controller->Initialize(Model);

	// State와 Model 생성
	TSharedPtr<FFXLibraryState> State = MakeShared<FFXLibraryState>();
	TSharedPtr<FFXLibraryModel> Model = MakeShared<FFXLibraryModel>();
	Model->SetState(State);  // ⭐ State를 Model에 설정

	// Controller 생성 및 초기화
	Controller = MakeShared<SFXAssetRegistPanelController>();
	Controller->Initialize(Model);

	// 초기 데이터 로드
	if (Model.IsValid())
	{
		Model->LoadFromSettings();
	}

	// 기본값 설정
	RootPath = FFXAssetLibConstants::DefaultRootPath;
	
	// 첫 번째 선택된 에셋의 이름을 기본값으로 사용
	if (SelectedAssets.Num() > 0)
	{
		AssetName = SelectedAssets[0].AssetName.ToString();
		CategoryName = FFXAssetLibConstants::DefaultCategoryName;
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
	if (!Controller.IsValid())
	{
		return FReply::Handled();
	}

	FReply Result = Controller->OnRegisterClicked(RootPath, AssetName, CategoryName, SelectedAssets);

	// 창 닫기
	if (TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared()))
	{
		ParentWindow->RequestDestroyWindow();
	}

	return Result;
}

FReply SFXAssetRegistPanel::OnCancelClicked()
{
	if (Controller.IsValid())
	{
		Controller->OnCancelClicked();
	}

	// 창 닫기
	if (TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared()))
	{
		ParentWindow->RequestDestroyWindow();
	}
	return FReply::Handled();
}

void SFXAssetRegistPanel::OnRootPathChanged(const FText& NewText)
{
	if (Controller.IsValid())
	{
		Controller->OnRootPathChanged(NewText, RootPath);
	}
	else
	{
		RootPath = NewText.ToString();
	}
}

void SFXAssetRegistPanel::OnAssetNameChanged(const FText& NewText)
{
	if (Controller.IsValid())
	{
		Controller->OnAssetNameChanged(NewText, AssetName);
	}
	else
	{
		AssetName = NewText.ToString();
	}
}

void SFXAssetRegistPanel::OnCategorySelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (Controller.IsValid())
	{
		Controller->OnCategorySelectionChanged(NewSelection, CategoryName);
		SelectedCategoryOption = NewSelection;
	}
	else if (NewSelection.IsValid())
	{
		CategoryName = *NewSelection;
		SelectedCategoryOption = NewSelection;
	}
}

FReply SFXAssetRegistPanel::OnAddCategoryClicked()
{
	// 간단한 입력 다이얼로그 생성
	FText Title = FText::FromString(TEXT("New Category"));
	FText Message = FText::FromString(TEXT("Enter category name:"));
	FString DefaultValue = TEXT("NewCategory");
	
	// 입력 다이얼로그 창 생성
	TSharedRef<SWindow> InputWindow = SNew(SWindow)
		.Title(Title)
		.SizingRule(ESizingRule::UserSized)
		.ClientSize(FFXAssetLibConstants::RegistrationWindowSize)
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
					if (!NewCategoryName.IsEmpty() && Controller.IsValid())
					{
						// Controller를 통해 카테고리 추가
						TSharedPtr<FFXLibraryModel> Model = Controller->GetModel();
						if (Model.IsValid())
						{
							Model->AddCategory(*NewCategoryName);
							
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
										SelectedCategoryOption = Option;
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
	
	if (Controller.IsValid())
	{
		Controller->LoadCategoryOptions(CategoryOptions);
	}
	else
	{
		// 기본값이 없으면 "Default" 추가
		CategoryOptions.Add(MakeShareable(new FString(FFXAssetLibConstants::DefaultCategoryName)));
	}
	
	// 현재 선택된 카테고리가 있으면 선택 유지
	if (SelectedCategoryOption.IsValid())
	{
		FString SelectedName = *SelectedCategoryOption;
		for (const TSharedPtr<FString>& Option : CategoryOptions)
		{
			if (Option.IsValid() && *Option == SelectedName)
			{
				if (CategoryComboBox.IsValid())
				{
					CategoryComboBox->SetSelectedItem(Option);
				}
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
	if (Controller.IsValid())
	{
		Controller->OnHashtagsChanged(NewText, Hashtags);
	}
	else
	{
		Hashtags = NewText.ToString();
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

