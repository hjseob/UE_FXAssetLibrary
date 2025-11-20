// Fill out your copyright notice in the Description page of Project Settings.

#include "View/SFXLibraryPanel.h"
#include "SlateOptMacros.h"

// Model & Controller
#include "Model/FXLibraryState.h"
#include "Model/FXLibraryModel.h"
#include "Controller/SFXLibraryPanelController.h"

// Utils
#include "Utils/FXUIUtils.h"
#include "Core/FXAssetLibConstants.h"

// Widgets
#include "View/Widgets/SFXCategoryRowWidget.h"
#include "View/Widgets/SFXAssetTileWidget.h"

// Slate Widgets
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/STableRow.h"

// Editor
#include "Editor.h"

// Asset Registry
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetThumbnail.h"

// App Style
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SFXLibraryPanel::Construct(const FArguments& InArgs)
{
	// State와 Model 생성
	State = MakeShared<FFXLibraryState>();
	TSharedPtr<FFXLibraryModel> Model = MakeShared<FFXLibraryModel>();
	
	// Controller 생성 및 초기화
	Controller = MakeShared<SFXLibraryPanelController>();
	Controller->Initialize(State, Model);

	// State 변경 델리게이트 바인딩
	BindStateDelegates();

	// 초기 데이터 로드
	if (Model.IsValid())
	{
		Model->LoadFromSettings();
	}

	// ⭐ 여기에 추가: ListItemsSource 포인터 초기화
	if (State.IsValid())
	{
		CategoryListSource = &State->Categories;
		AssetTileListSource = &State->VisibleAssets;
	}

	// UI 구성
	ChildSlot
		[
			SNew(SVerticalBox)

				// 상단: 버튼 툴바
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)
				[
					SNew(SHorizontalBox)

						// 새로고침 버튼
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0, 0, 5, 0)
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("Refresh")))
								.ToolTipText(FText::FromString(TEXT("Reload assets from settings")))
								.OnClicked_Lambda([this]()
									{
										return Controller.IsValid() ? Controller->OnRefreshClicked() : FReply::Handled();
									})
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(SHorizontalBox)

										+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Center)
										.Padding(0, 0, 5, 0)
										[
											SNew(SBox)
												.WidthOverride(16)
												.HeightOverride(16)
												[
													SNew(SImage)
														.Image(FAppStyle::GetBrush("Icons.Refresh"))
												]
										]

										+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Center)
										[
											SNew(STextBlock)
												.Text(FText::FromString(TEXT("Refresh")))
										]
								]
						]

					// Reset Niagara 버튼
					+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0, 0, 5, 0)
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("Reset Selected")))
								.ToolTipText(FText::FromString(TEXT("Reset selected Niagara actors (Shift+/)")))
								.OnClicked_Lambda([this]()
									{
										return Controller.IsValid() ? Controller->OnResetNiagaraClicked() : FReply::Handled();
									})
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(SHorizontalBox)

										+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Center)
										.Padding(0, 0, 5, 0)
										[
											SNew(SBox)
												.WidthOverride(16)
												.HeightOverride(16)
												[
													SNew(SImage)
														.Image(FAppStyle::GetBrush("Icons.CircleArrowLeft"))
												]
										]

										+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Center)
										[
											SNew(STextBlock)
												.Text(FText::FromString(TEXT("Reset Selected")))
										]
								]
						]

					// Clean Up 버튼
					+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("Clean Up")))
								.ToolTipText(FText::FromString(TEXT("Clean up and organize registered assets and categories")))
								.OnClicked_Lambda([this]()
									{
										return Controller.IsValid() ? Controller->OnCleanupClicked() : FReply::Handled();
									})
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(SHorizontalBox)

										+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Center)
										.Padding(0, 0, 5, 0)
										[
											SNew(SBox)
												.WidthOverride(16)
												.HeightOverride(16)
												[
													SNew(SImage)
														.Image(FAppStyle::GetBrush("Icons.Delete"))
												]
										]

										+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Center)
										[
											SNew(STextBlock)
												.Text(FText::FromString(TEXT("Clean Up")))
										]
								]
						]
				]

			// 메인 컨텐츠
			+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(SSplitter)
						.Orientation(Orient_Horizontal)

						// 왼쪽: 카테고리
						+ SSplitter::Slot()
						.Value(0.3f)
						[
							SNew(SBorder)
								.Padding(5)
								[
									SNew(SVerticalBox)

										+ SVerticalBox::Slot()
										.AutoHeight()
										.Padding(5)
										[
											SNew(STextBlock)
												.Text(FText::FromString(TEXT("Categories")))
												.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
										]

										+ SVerticalBox::Slot()
										.FillHeight(1.0f)
										[
											SAssignNew(CategoryList, SListView<TSharedPtr<FName>>)
												.ListItemsSource(CategoryListSource)
												.OnGenerateRow(this, &SFXLibraryPanel::GenCatRow)
												.OnSelectionChanged_Lambda([this](TSharedPtr<FName> NewSel, ESelectInfo::Type SelectInfo)
													{
														if (Controller.IsValid() && NewSel.IsValid())
														{
															Controller->OnCategoryChanged(NewSel);
															RefreshUI();
														}
													})
												.ItemHeight(200)
										]
								]
						]

					// 오른쪽: 에셋 타일
					+ SSplitter::Slot()
						.Value(0.7f)
						[
							SNew(SBorder)
								.Padding(5)
								[
									SNew(SVerticalBox)

										+ SVerticalBox::Slot()
										.AutoHeight()
										.Padding(5)
										[
											SNew(STextBlock)
												.Text(FText::FromString(TEXT("Niagara Assets")))
												.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
										]

										+ SVerticalBox::Slot()
										.FillHeight(1.0f)
										[
											SAssignNew(AssetTileView, STileView<TSharedPtr<FSoftObjectPath>>)
												.ListItemsSource(AssetTileListSource)
												.OnGenerateTile(this, &SFXLibraryPanel::GenAssetTile)
												.ItemWidth(FFXAssetLibConstants::AssetTileSize.X)
												.ItemHeight(FFXAssetLibConstants::AssetTileSize.Y)
										]
								]
						]
				]
		];

	// 첫 번째 카테고리 선택
	if (State.IsValid() && State->Categories.Num() > 0)
	{
		CategoryList->SetSelection(State->Categories[0]);
		Controller->OnCategoryChanged(State->Categories[0]);
		RefreshUI();
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SFXLibraryPanel::BindStateDelegates()
{
	if (State.IsValid())
	{
		State->OnStateChanged.AddLambda([this]()
			{
				RefreshUI();
			});
	}
}

void SFXLibraryPanel::RefreshUI()
{
	if (CategoryList.IsValid())
	{
		CategoryList->RequestListRefresh();
	}

	if (AssetTileView.IsValid())
	{
		AssetTileView->RequestListRefresh();
	}
}

TSharedRef<ITableRow> SFXLibraryPanel::GenCatRow(TSharedPtr<FName> Item, const TSharedRef<STableViewBase>& Owner)
{
	if (!Item.IsValid() || !State.IsValid())
	{
		return SNew(STableRow<TSharedPtr<FName>>, Owner);
	}

	FName CategoryName = *Item;

	// 배경 이미지 브러시 로드
	FSlateBrush* BackgroundBrush = nullptr;
	const FSoftObjectPath* IconPath = State->CategoryIcons.Find(CategoryName);

	if (IconPath && IconPath->IsValid())
	{
		UObject* LoadedObject = IconPath->TryLoad();
		if (LoadedObject)
		{
			UTexture2D* BgTexture = Cast<UTexture2D>(LoadedObject);
			if (BgTexture)
			{
				BackgroundBrush = new FSlateDynamicImageBrush(
					BgTexture,
					FFXAssetLibConstants::CategoryTileSize,
					BgTexture->GetFName()
				);
			}
		}
	}

	if (!BackgroundBrush)
	{
		BackgroundBrush = new FSlateColorBrush(FLinearColor(0.15f, 0.15f, 0.2f, 1.0f));
	}

	return SNew(SFXCategoryRowWidget, Owner)
		.CategoryName(Item)
		.BackgroundBrush(BackgroundBrush)
		.ParentPanel(this)
		.HoveredCategory(State->HoveredCategory);
}

TSharedRef<ITableRow> SFXLibraryPanel::GenAssetTile(TSharedPtr<FSoftObjectPath> Item, const TSharedRef<STableViewBase>& Owner)
{
	if (!Item.IsValid())
	{
		return SNew(STableRow<TSharedPtr<FSoftObjectPath>>, Owner);
	}

	FString AssetName = Item->GetAssetName();

	// Asset Registry에서 FAssetData 가져오기
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(*Item);

	// 썸네일 생성
	TSharedPtr<FAssetThumbnail> Thumbnail = FFXUIUtils::CreateAssetThumbnail(
		AssetData,
		FFXAssetLibConstants::ThumbnailSize.X,
		FFXAssetLibConstants::ThumbnailSize.Y,
		GetThumbnailPool()
	);

	// 타일 위젯 생성
	TSharedRef<SWidget> TileWidget = SNew(SFXAssetTileWidget)
		.AssetPath(Item)
		.ParentPanel(this)
		.Thumbnail(Thumbnail)
		.AssetName(AssetName);

	// 좌클릭으로 스폰
	return SNew(STableRow<TSharedPtr<FSoftObjectPath>>, Owner)
		.Padding(2)
		[
			SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "NoBorder")
				.OnClicked_Lambda([this, Item]()
					{
						if (Controller.IsValid())
						{
							Controller->SpawnNiagaraActor(Item);
						}
						return FReply::Handled();
					})
				[
					TileWidget
				]
		];
}

TSharedPtr<FAssetThumbnailPool> SFXLibraryPanel::GetThumbnailPool()
{
	if (!ThumbnailPool.IsValid())
	{
		ThumbnailPool = FFXUIUtils::CreateThumbnailPool(FFXAssetLibConstants::ThumbnailPoolSize);
	}
	return ThumbnailPool;
}

void SFXLibraryPanel::OnCategoryHovered(TSharedPtr<FName> Category)
{
	if (Controller.IsValid())
	{
		Controller->OnCategoryHovered(Category);
	}
}

void SFXLibraryPanel::OnCategoryUnhovered()
{
	if (Controller.IsValid())
	{
		Controller->OnCategoryUnhovered();
	}
}

void SFXLibraryPanel::OnBrowseToAsset(TSharedPtr<FSoftObjectPath> AssetPath)
{
	if (Controller.IsValid())
	{
		Controller->BrowseToAsset(AssetPath);
	}
}

void SFXLibraryPanel::OnRemoveAssetFromLibrary(TSharedPtr<FSoftObjectPath> AssetPath)
{
	if (Controller.IsValid())
	{
		Controller->RemoveAssetFromLibrary(AssetPath);
		RefreshUI();
	}
}

void SFXLibraryPanel::OnCatChanged(TSharedPtr<FName> NewSel, ESelectInfo::Type SelectInfo)
{
	if (Controller.IsValid() && NewSel.IsValid())
	{
		Controller->OnCategoryChanged(NewSel);
		RefreshUI();
	}
}

