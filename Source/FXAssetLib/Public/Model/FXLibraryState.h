// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"

/**
 * FX Library 상태 관리 클래스
 * 모든 UI 상태를 중앙에서 관리하는 State 패턴 구현
 */
class FXASSETLIB_API FFXLibraryState
{
public:
	FFXLibraryState();
	~FFXLibraryState();

	// 상태 변경 델리게이트
	DECLARE_MULTICAST_DELEGATE(FOnStateChanged);
	FOnStateChanged OnStateChanged;

	// 카테고리 관련 상태
	TArray<TSharedPtr<FName>> Categories;
	TMap<FName, TArray<FSoftObjectPath>> CategoryToAssets;
	TMap<FName, FSoftObjectPath> CategoryIcons;
	TSharedPtr<FName> SelectedCategory;
	TSharedPtr<FName> HoveredCategory;

	// 현재 표시 중인 에셋 목록
	TArray<TSharedPtr<FSoftObjectPath>> VisibleAssets;

	// UI 상태
	bool bIsCategoryModalOpen = false;
	bool bIsLoading = false;

	// 액션: 카테고리 선택
	void SetSelectedCategory(TSharedPtr<FName> Category);
	void SetHoveredCategory(TSharedPtr<FName> Category);

	// 액션: 카테고리 모달
	void OpenCategoryModal();
	void CloseCategoryModal();

	// 액션: 데이터 로드
	void LoadCategories(const TArray<TSharedPtr<FName>>& InCategories);
	void LoadCategoryAssets(FName CategoryName, const TArray<FSoftObjectPath>& Assets);
	void LoadCategoryIcon(FName CategoryName, const FSoftObjectPath& IconPath);
	void UpdateVisibleAssets();

	// 액션: 에셋 관리
	void AddAssetToCategory(FName CategoryName, const FSoftObjectPath& AssetPath);
	void RemoveAssetFromCategory(FName CategoryName, const FSoftObjectPath& AssetPath);

	// 상태 초기화
	void Reset();

	void NotifyStateChanged();
};

