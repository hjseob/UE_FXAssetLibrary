// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


// ⭐ Forward declarations (불필요한 include 방지)
struct FAssetData;
//class SComboBox;
class SEditableTextBox;


/**
 * FX Asset Registration Panel
 * 나이아가라 에셋 등록을 위한 독립 창
 */
class FXASSETLIB_API SFXAssetRegistPanel : public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SFXAssetRegistPanel) {}
		SLATE_ARGUMENT(TArray<FAssetData>, SelectedAssets)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// 입력 필드들
	FString RootPath;
	FString AssetName;
	FString CategoryName;
	FString Hashtags;

	// 선택된 에셋들
	TArray<FAssetData> SelectedAssets;

	// 카테고리 목록
	TArray<TSharedPtr<FString>> CategoryOptions;
	TSharedPtr<FString> SelectedCategoryOption;
	TSharedPtr<class SComboBox<TSharedPtr<FString>>> CategoryComboBox;

	// 콜백 함수들
	FReply OnRegisterClicked();
	FReply OnCancelClicked();
	FReply OnAddCategoryClicked();
	void OnRootPathChanged(const FText& NewText);
	void OnAssetNameChanged(const FText& NewText);
	void OnCategorySelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	void OnHashtagsChanged(const FText& NewText);

	// 카테고리 목록 로드
	void LoadCategoryOptions();
	TSharedRef<SWidget> MakeCategoryWidget(TSharedPtr<FString> InOption);
	FText GetCategoryComboBoxContent() const;
};