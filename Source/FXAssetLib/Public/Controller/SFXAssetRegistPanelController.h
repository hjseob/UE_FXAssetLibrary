// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"

// Forward declarations
class FFXLibraryModel;

/**
 * FX Asset Registration Panel Controller
 * SFXAssetRegistPanel의 이벤트 핸들러를 처리
 */
class FXASSETLIB_API SFXAssetRegistPanelController
{
public:
	SFXAssetRegistPanelController();
	~SFXAssetRegistPanelController();

	// 초기화
	void Initialize(TSharedPtr<FFXLibraryModel> InModel);

	// 버튼 클릭 이벤트
	FReply OnRegisterClicked(
		const FString& RootPath,
		const FString& AssetName,
		const FString& CategoryName,
		const TArray<FAssetData>& SelectedAssets
	);
	FReply OnCancelClicked();

	// 입력 필드 변경 이벤트
	void OnRootPathChanged(const FText& NewText, FString& OutRootPath);
	void OnAssetNameChanged(const FText& NewText, FString& OutAssetName);
	void OnCategorySelectionChanged(TSharedPtr<FString> NewSelection, FString& OutCategoryName);
	void OnHashtagsChanged(const FText& NewText, FString& OutHashtags);

	// 카테고리 관리
	FReply OnAddCategoryClicked(FString& OutNewCategoryName);
	void LoadCategoryOptions(TArray<TSharedPtr<FString>>& OutCategoryOptions);

	// Model 접근
	TSharedPtr<FFXLibraryModel> GetModel() const { return Model; }

private:
	TSharedPtr<FFXLibraryModel> Model;
};

