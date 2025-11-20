// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STableRow.h"
#include "UObject/SoftObjectPath.h"

// Forward declarations
class SFXLibraryPanel;

/**
 * 카테고리 행 위젯
 * 카테고리 리스트에서 각 카테고리를 표시하는 위젯
 */
class FXASSETLIB_API SFXCategoryRowWidget : public STableRow<TSharedPtr<FName>>
{
public:
	SLATE_BEGIN_ARGS(SFXCategoryRowWidget) {}
		SLATE_ARGUMENT(TSharedPtr<FName>, CategoryName)
		SLATE_ARGUMENT(FSlateBrush*, BackgroundBrush)
		SLATE_ARGUMENT(SFXLibraryPanel*, ParentPanel)
		SLATE_ARGUMENT(TSharedPtr<FName>, HoveredCategory)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable);

	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

private:
	TSharedPtr<FName> CategoryName;
	FSlateBrush* BackgroundBrush;
	SFXLibraryPanel* ParentPanel;
	TSharedPtr<FName> HoveredCategory;
};

