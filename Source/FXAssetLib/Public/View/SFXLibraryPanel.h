// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STileView.h"
#include "Widgets/Views/STableViewBase.h"
#include "UObject/SoftObjectPath.h"

// Forward declarations
class FFXLibraryState;
class SFXLibraryPanelController;
class SFXCategoryModalWidget;

/**
 * FX Library Panel View
 * UI 구성만 담당하는 View 클래스
 */
class FXASSETLIB_API SFXLibraryPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFXLibraryPanel) {}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	// Controller에서 호출하는 콜백 (위젯에서 사용)
	void OnCategoryHovered(TSharedPtr<FName> Category);
	void OnCategoryUnhovered();
	void OnBrowseToAsset(TSharedPtr<FSoftObjectPath> AssetPath);
	void OnRemoveAssetFromLibrary(TSharedPtr<FSoftObjectPath> AssetPath);
	void OnCategorySelected(TSharedPtr<FName> Category);
	void CloseCategorySubWidget();

	// State 변경 시 UI 업데이트
	void RefreshUI();

private:
	// UI 생성 콜백
	TSharedRef<ITableRow> GenAssetTile(TSharedPtr<FSoftObjectPath> Item, const TSharedRef<STableViewBase>& Owner);

	// 썸네일 풀 가져오기
	TSharedPtr<class FAssetThumbnailPool> GetThumbnailPool();

	// 카테고리 버튼 이벤트 처리
	FReply OnCategoryButtonMouseDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	FReply OnCategoryButtonMouseUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

	// State 변경 델리게이트 바인딩
	void BindStateDelegates();

private:
	// State와 Controller
	TSharedPtr<FFXLibraryState> State;
	TSharedPtr<SFXLibraryPanelController> Controller;

	// UI 위젯
	TSharedPtr<STileView<TSharedPtr<FSoftObjectPath>>> AssetTileView;
	TSharedPtr<class SBorder> CategoryButton;
	TSharedPtr<SFXCategoryModalWidget> CategorySubWidget;

	// 썸네일 풀
	TSharedPtr<class FAssetThumbnailPool> ThumbnailPool;

	// ListItemsSource용 포인터 (State 변경 시 업데이트)
	TArray<TSharedPtr<FSoftObjectPath>>* AssetTileListSource = nullptr;

	// 서브위젯 관련
	bool bIsSubWidgetVisible = false;
};

