// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FXLibrarySettings.h"
#include "Model/FXLibraryState.h"

/**
 * FX Library 데이터 모델
 * UFXLibrarySettings와 FFXLibraryState 사이의 중재자
 * 데이터 로드, 저장, 검증 로직 담당
 */
class FXASSETLIB_API FFXLibraryModel
{
public:
	FFXLibraryModel();
	~FFXLibraryModel();

	// State 참조 설정
	void SetState(TSharedPtr<FFXLibraryState> InState);

	// Settings에서 State로 데이터 로드
	void LoadFromSettings();

	// State에서 Settings로 데이터 저장
	void SaveToSettings();

	// 카테고리 관리
	FFXCategoryData* FindCategory(FName CategoryName);
	FFXCategoryData* AddCategory(FName CategoryName, const FSoftObjectPath& IconPath = FSoftObjectPath());
	bool RemoveCategory(FName CategoryName);

	// 에셋 관리
	bool AddAssetToCategory(FName CategoryName, const FSoftObjectPath& AssetPath);
	int32 RemoveAssetFromCategory(FName CategoryName, const FSoftObjectPath& AssetPath);

	// 검증
	bool ValidateCategory(FName CategoryName) const;
	bool ValidateAsset(const FSoftObjectPath& AssetPath) const;

	// 정리 (유효하지 않은 에셋/카테고리 제거)
	int32 CleanupInvalidAssets();
	int32 CleanupEmptyCategories();

	// State 접근
	TSharedPtr<FFXLibraryState> GetState() const { return State; }

private:
	TSharedPtr<FFXLibraryState> State;
	UFXLibrarySettings* GetMutableSettings() const;
	const UFXLibrarySettings* GetSettings() const;
};

