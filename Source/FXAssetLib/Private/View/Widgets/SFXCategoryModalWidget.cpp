// Fill out your copyright notice in the Description page of Project Settings.

#include "View/Widgets/SFXCategoryModalWidget.h"
#include "View/SFXLibraryPanel.h"
#include "View/Widgets/SFXCategoryRowWidget.h"
#include "Model/FXLibraryState.h"
#include "Core/FXAssetLibConstants.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Views/STableRow.h"
#include "Styling/CoreStyle.h"
#include "Styling/AppStyle.h"

void SFXCategoryModalWidget::Construct(const FArguments& InArgs)
{
	ParentPanel = InArgs._ParentPanel;
	State = InArgs._State;

	if (State.IsValid())
	{
		CategoryListSource = &State->Categories;
	}

	ChildSlot
		[
			SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("Menu.Background"))
				.Padding(5)
				[
					SNew(SBox)
						.WidthOverride(300)
						.HeightOverride(400)
						[
							SAssignNew(CategoryListView, SListView<TSharedPtr<FName>>)
								.ListItemsSource(CategoryListSource)
								.OnGenerateRow(this, &SFXCategoryModalWidget::GenerateCategoryRow)
								.ItemHeight(120)
						]
				]
		];
}

TSharedRef<ITableRow> SFXCategoryModalWidget::GenerateCategoryRow(TSharedPtr<FName> Item, const TSharedRef<STableViewBase>& Owner)
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
		.ParentPanel(ParentPanel)
		.HoveredCategory(State->HoveredCategory);
}

