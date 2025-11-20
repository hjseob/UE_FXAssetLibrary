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
#include "View/Widgets/SFXCategoryModalWidget.h"

// Slate Widgets
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/STableRow.h"
#include "Framework/Application/SlateApplication.h"

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
					SNew(SBorder)
						.Padding(5)
						[
							SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(5)
								[
									SNew(SHorizontalBox)

										+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Center)
										[
											SNew(STextBlock)
												.Text(FText::FromString(TEXT("Niagara Assets")))
												.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
										]

										+ SHorizontalBox::Slot()
										.AutoWidth()
										.Padding(10, 0, 0, 0)
										.VAlign(VAlign_Center)
										[
											SAssignNew(CategoryButton, SBorder)
												.BorderImage(FAppStyle::GetBrush("Button"))
												.Padding(FMargin(8, 4))
												.OnMouseButtonDown(this, &SFXLibraryPanel::OnCategoryButtonMouseDown)
												.OnMouseButtonUp(this, &SFXLibraryPanel::OnCategoryButtonMouseUp)
												.Cursor(EMouseCursor::Hand)
												.ToolTipText(FText::FromString(TEXT("Click and hold to select category")))
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
																		.Image(FAppStyle::GetBrush("Icons.Folder"))
																]
														]

														+ SHorizontalBox::Slot()
														.AutoWidth()
														.VAlign(VAlign_Center)
														[
															SNew(STextBlock)
																.Text_Lambda([this]()
																	{
																		if (State.IsValid() && State->SelectedCategory.IsValid())
																		{
																			return FText::FromName(*State->SelectedCategory);
																		}
																		return FText::FromString(TEXT("Select Category"));
																	})
														]
												]
										]
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
		];

	// 첫 번째 카테고리 선택
	if (State.IsValid() && State->Categories.Num() > 0)
	{
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
	if (AssetTileView.IsValid())
	{
		AssetTileView->RequestListRefresh();
	}
	if (CategoryButton.IsValid())
	{
		CategoryButton->Invalidate(EInvalidateWidgetReason::Paint);
	}
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

FReply SFXLibraryPanel::OnCategoryButtonMouseDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && !bIsModalOpen && CategoryButton.IsValid())
	{
		// 모달 위젯 생성
		CategoryModalContent = SNew(SFXCategoryModalWidget)
			.ParentPanel(this)
			.State(State);

		// 모달 표시 - 간단하고 안정적인 형태
		FSlateApplication::Get().PushMenu(
			CategoryButton.ToSharedRef(),
			FWidgetPath(),
			CategoryModalContent.ToSharedRef(),
			MouseEvent.GetScreenSpacePosition(),
			FPopupTransitionEffect(FPopupTransitionEffect::None)
		);

		bIsModalOpen = true;
		return FReply::Handled().CaptureMouse(CategoryButton.ToSharedRef());
	}

	return FReply::Unhandled();
}

FReply SFXLibraryPanel::OnCategoryButtonMouseUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsModalOpen)
	{
		// 모달이 열려있으면 닫기
		// 카테고리 선택은 SFXCategoryRowWidget::OnMouseButtonUp에서 처리되며,
		// 그곳에서 CloseCategoryModal()을 호출함
		// 여기서는 버튼에서 직접 릴리즈된 경우를 처리
		CloseCategoryModal();
		return FReply::Handled().ReleaseMouseCapture();
	}

	return FReply::Unhandled();
}

void SFXLibraryPanel::CloseCategoryModal()
{
	if (bIsModalOpen)
	{
		FSlateApplication::Get().DismissAllMenus();
		bIsModalOpen = false;
		CategoryModalContent.Reset();
	}
}

void SFXLibraryPanel::OnCategorySelected(TSharedPtr<FName> Category)
{

	if (Controller.IsValid() && Category.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("OnCategorySelected called with category: %s"), *Category->ToString());

		// 1. Controller를 통해 카테고리 변경 (State 업데이트)
		Controller->OnCategoryChanged(Category);

		// 2. State 업데이트 확인 및 로그
		if (State.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("State Updated - SelectedCategory: %s, VisibleAssets count: %d"),
				State->SelectedCategory.IsValid() ? *State->SelectedCategory->ToString() : TEXT("Invalid"),
				State->VisibleAssets.Num());

			// 3. 델리게이트를 통해 UI 새로고침 (중요: 이것이 RefreshUI를 트리거)
			State->OnStateChanged.Broadcast();
		}
	}

	// 4. 모달 닫기
	CloseCategoryModal();
}

