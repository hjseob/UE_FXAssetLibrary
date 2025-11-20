// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableViewBase.h"

// Forward declarations
class SFXLibraryPanel;
class FFXLibraryState;

/**
 * 카테고리 모달 위젯
 * 버튼을 누르고 있는 동안 카테고리 목록을 표시하는 모달
 */
class FXASSETLIB_API SFXCategoryModalWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFXCategoryModalWidget) {}
		SLATE_ARGUMENT(SFXLibraryPanel*, ParentPanel)
		SLATE_ARGUMENT(TSharedPtr<FFXLibraryState>, State)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// 카테고리 행 생성
	TSharedRef<ITableRow> GenerateCategoryRow(TSharedPtr<FName> Item, const TSharedRef<STableViewBase>& Owner);

private:
	SFXLibraryPanel* ParentPanel;
	TSharedPtr<FFXLibraryState> State;
	TSharedPtr<SListView<TSharedPtr<FName>>> CategoryListView;
	TArray<TSharedPtr<FName>>* CategoryListSource = nullptr;
};

