// Fill out your copyright notice in the Description page of Project Settings.

#include "Model/FXLibraryModel.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"

FFXLibraryModel::FFXLibraryModel()
{
}

FFXLibraryModel::~FFXLibraryModel()
{
}

void FFXLibraryModel::SetState(TSharedPtr<FFXLibraryState> InState)
{
	State = InState;
}

void FFXLibraryModel::LoadFromSettings()
{
	if (!State.IsValid())
	{
		return;
	}

	const UFXLibrarySettings* Settings = GetSettings();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("[FX Library Model] LoadFromSettings: Settings is nullptr!"));
		return;
	}

	// State 초기화
	State->Reset();

	// 카테고리 데이터 로드
	TArray<TSharedPtr<FName>> Categories;
	for (const FFXCategoryData& CategoryData : Settings->Categories)
	{
		// 카테고리 이름
		Categories.Add(MakeShared<FName>(CategoryData.CategoryName));

		// 에셋 목록
		State->LoadCategoryAssets(CategoryData.CategoryName, CategoryData.Assets);

		// 아이콘 경로
		if (CategoryData.IconPath.IsValid())
		{
			State->LoadCategoryIcon(CategoryData.CategoryName, CategoryData.IconPath);
		}
	}

	State->LoadCategories(Categories);

	// 첫 번째 카테고리 선택
	if (Categories.Num() > 0)
	{
		State->SetSelectedCategory(Categories[0]);
	}

	UE_LOG(LogTemp, Log, TEXT("[FX Library Model] Loaded %d categories"), Categories.Num());
}

void FFXLibraryModel::SaveToSettings()
{
	if (!State.IsValid())
	{
		return;
	}

	UFXLibrarySettings* Settings = GetMutableSettings();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("[FX Library Model] SaveToSettings: Settings is nullptr!"));
		return;
	}

	// State에서 Settings로 데이터 저장
	Settings->Categories.Empty();

	for (const TSharedPtr<FName>& CategoryPtr : State->Categories)
	{
		if (!CategoryPtr.IsValid())
		{
			continue;
		}

		FName CategoryName = *CategoryPtr;
		FFXCategoryData CategoryData;
		CategoryData.CategoryName = CategoryName;

		// 아이콘 경로
		if (const FSoftObjectPath* IconPath = State->CategoryIcons.Find(CategoryName))
		{
			CategoryData.IconPath = *IconPath;
		}

		// 에셋 목록
		if (const TArray<FSoftObjectPath>* Assets = State->CategoryToAssets.Find(CategoryName))
		{
			CategoryData.Assets = *Assets;
		}

		Settings->Categories.Add(CategoryData);
	}

	Settings->SaveConfig();
	Settings->TryUpdateDefaultConfigFile();

	UE_LOG(LogTemp, Log, TEXT("[FX Library Model] Saved %d categories"), Settings->Categories.Num());
}

FFXCategoryData* FFXLibraryModel::FindCategory(FName CategoryName)
{
	UFXLibrarySettings* Settings = GetMutableSettings();
	return Settings ? Settings->FindCategory(CategoryName) : nullptr;
}

FFXCategoryData* FFXLibraryModel::AddCategory(FName CategoryName, const FSoftObjectPath& IconPath)
{
	UFXLibrarySettings* Settings = GetMutableSettings();
	if (!Settings)
	{
		return nullptr;
	}

	FFXCategoryData* Category = Settings->AddCategory(CategoryName, IconPath);
	if (Category)
	{
		// State에도 추가
		TSharedPtr<FName> CategoryPtr = MakeShared<FName>(CategoryName);
		State->Categories.AddUnique(CategoryPtr);
		if (IconPath.IsValid())
		{
			State->LoadCategoryIcon(CategoryName, IconPath);
		}
		State->NotifyStateChanged();
	}

	return Category;
}

bool FFXLibraryModel::RemoveCategory(FName CategoryName)
{
	UFXLibrarySettings* Settings = GetMutableSettings();
	if (!Settings)
	{
		return false;
	}

	// Settings에서 제거
	int32 RemovedCount = Settings->Categories.RemoveAll([CategoryName](const FFXCategoryData& Cat)
	{
		return Cat.CategoryName == CategoryName;
	});

	if (RemovedCount > 0)
	{
		// State에서도 제거
		State->Categories.RemoveAll([CategoryName](const TSharedPtr<FName>& CatPtr)
		{
			return CatPtr.IsValid() && *CatPtr == CategoryName;
		});
		State->CategoryToAssets.Remove(CategoryName);
		State->CategoryIcons.Remove(CategoryName);

		// 선택된 카테고리였다면 해제
		if (State->SelectedCategory.IsValid() && *State->SelectedCategory == CategoryName)
		{
			State->SetSelectedCategory(nullptr);
		}

		State->NotifyStateChanged();
		SaveToSettings();
	}

	return RemovedCount > 0;
}

bool FFXLibraryModel::AddAssetToCategory(FName CategoryName, const FSoftObjectPath& AssetPath)
{
	UFXLibrarySettings* Settings = GetMutableSettings();
	if (!Settings)
	{
		return false;
	}

	bool bSuccess = Settings->AddAssetToCategory(CategoryName, AssetPath);
	if (bSuccess)
	{
		State->AddAssetToCategory(CategoryName, AssetPath);
		SaveToSettings();
	}

	return bSuccess;
}

int32 FFXLibraryModel::RemoveAssetFromCategory(FName CategoryName, const FSoftObjectPath& AssetPath)
{
	UFXLibrarySettings* Settings = GetMutableSettings();
	if (!Settings)
	{
		return 0;
	}

	int32 RemovedCount = Settings->RemoveAssetFromCategory(CategoryName, AssetPath);
	if (RemovedCount > 0)
	{
		State->RemoveAssetFromCategory(CategoryName, AssetPath);
		SaveToSettings();
	}

	return RemovedCount;
}

bool FFXLibraryModel::ValidateCategory(FName CategoryName) const
{
	const UFXLibrarySettings* Settings = GetSettings();
	return Settings && Settings->FindCategory(CategoryName) != nullptr;
}

bool FFXLibraryModel::ValidateAsset(const FSoftObjectPath& AssetPath) const
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(*AssetPath.ToString());
	return AssetData.IsValid();
}

int32 FFXLibraryModel::CleanupInvalidAssets()
{
	if (!State.IsValid())
	{
		return 0;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	int32 RemovedCount = 0;

	// 각 카테고리에서 유효하지 않은 에셋 제거
	for (const TSharedPtr<FName>& CategoryPtr : State->Categories)
	{
		if (!CategoryPtr.IsValid())
		{
			continue;
		}

		FName CategoryName = *CategoryPtr;
		if (TArray<FSoftObjectPath>* Assets = State->CategoryToAssets.Find(CategoryName))
		{
			TArray<FSoftObjectPath> ValidAssets;
			for (const FSoftObjectPath& AssetPath : *Assets)
			{
				FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(*AssetPath.ToString());
				if (AssetData.IsValid())
				{
					ValidAssets.Add(AssetPath);
				}
				else
				{
					RemovedCount++;
					UE_LOG(LogTemp, Log, TEXT("[FX Library Model] Removed invalid asset: %s"), *AssetPath.ToString());
				}
			}
			*Assets = ValidAssets;
		}
	}

	if (RemovedCount > 0)
	{
		State->UpdateVisibleAssets();
		State->NotifyStateChanged();
		SaveToSettings();
	}

	return RemovedCount;
}

int32 FFXLibraryModel::CleanupEmptyCategories()
{
	if (!State.IsValid())
	{
		return 0;
	}

	int32 RemovedCount = 0;
	TArray<TSharedPtr<FName>> ValidCategories;

	for (const TSharedPtr<FName>& CategoryPtr : State->Categories)
	{
		if (!CategoryPtr.IsValid())
		{
			continue;
		}

		FName CategoryName = *CategoryPtr;
		const TArray<FSoftObjectPath>* Assets = State->CategoryToAssets.Find(CategoryName);

		if (Assets && Assets->Num() > 0)
		{
			ValidCategories.Add(CategoryPtr);
		}
		else
		{
			RemovedCount++;
			UE_LOG(LogTemp, Log, TEXT("[FX Library Model] Removed empty category: %s"), *CategoryName.ToString());
		}
	}

	if (RemovedCount > 0)
	{
		State->Categories = ValidCategories;
		State->NotifyStateChanged();
		SaveToSettings();
	}

	return RemovedCount;
}

UFXLibrarySettings* FFXLibraryModel::GetMutableSettings() const
{
	return GetMutableDefault<UFXLibrarySettings>();
}

const UFXLibrarySettings* FFXLibraryModel::GetSettings() const
{
	return GetDefault<UFXLibrarySettings>();
}

