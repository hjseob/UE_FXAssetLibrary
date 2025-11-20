// Fill out your copyright notice in the Description page of Project Settings.

#include "Model/FXLibraryState.h"

FFXLibraryState::FFXLibraryState()
{
	Reset();
}

FFXLibraryState::~FFXLibraryState()
{
}

void FFXLibraryState::SetSelectedCategory(TSharedPtr<FName> Category)
{
	SelectedCategory = Category;
	UpdateVisibleAssets();
	NotifyStateChanged();
}

void FFXLibraryState::SetHoveredCategory(TSharedPtr<FName> Category)
{
	HoveredCategory = Category;
	NotifyStateChanged();
}

void FFXLibraryState::OpenCategoryModal()
{
	bIsCategoryModalOpen = true;
	NotifyStateChanged();
}

void FFXLibraryState::CloseCategoryModal()
{
	bIsCategoryModalOpen = false;
	NotifyStateChanged();
}

void FFXLibraryState::LoadCategories(const TArray<TSharedPtr<FName>>& InCategories)
{
	Categories = InCategories;
	NotifyStateChanged();
}

void FFXLibraryState::LoadCategoryAssets(FName CategoryName, const TArray<FSoftObjectPath>& Assets)
{
	CategoryToAssets.Add(CategoryName, Assets);
	UpdateVisibleAssets();
	NotifyStateChanged();
}

void FFXLibraryState::LoadCategoryIcon(FName CategoryName, const FSoftObjectPath& IconPath)
{
	CategoryIcons.Add(CategoryName, IconPath);
	NotifyStateChanged();
}

void FFXLibraryState::UpdateVisibleAssets()
{
	VisibleAssets.Empty();

	if (SelectedCategory.IsValid())
	{
		if (const TArray<FSoftObjectPath>* Assets = CategoryToAssets.Find(*SelectedCategory))
		{
			for (const FSoftObjectPath& Path : *Assets)
			{
				VisibleAssets.Add(MakeShared<FSoftObjectPath>(Path));
			}
		}
	}
}

void FFXLibraryState::AddAssetToCategory(FName CategoryName, const FSoftObjectPath& AssetPath)
{
	if (TArray<FSoftObjectPath>* Assets = CategoryToAssets.Find(CategoryName))
	{
		Assets->AddUnique(AssetPath);
	}
	else
	{
		CategoryToAssets.Add(CategoryName, { AssetPath });
	}
	UpdateVisibleAssets();
	NotifyStateChanged();
}

void FFXLibraryState::RemoveAssetFromCategory(FName CategoryName, const FSoftObjectPath& AssetPath)
{
	if (TArray<FSoftObjectPath>* Assets = CategoryToAssets.Find(CategoryName))
	{
		Assets->Remove(AssetPath);
		UpdateVisibleAssets();
		NotifyStateChanged();
	}
}

void FFXLibraryState::Reset()
{
	Categories.Empty();
	CategoryToAssets.Empty();
	CategoryIcons.Empty();
	SelectedCategory.Reset();
	HoveredCategory.Reset();
	VisibleAssets.Empty();
	bIsCategoryModalOpen = false;
	bIsLoading = false;
}

void FFXLibraryState::NotifyStateChanged()
{
	OnStateChanged.Broadcast();
}

