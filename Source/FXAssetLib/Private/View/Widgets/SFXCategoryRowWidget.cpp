// Fill out your copyright notice in the Description page of Project Settings.

#include "View/Widgets/SFXCategoryRowWidget.h"
#include "View/SFXLibraryPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
//#include "Widgets/Layout/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Styling/CoreStyle.h"

void SFXCategoryRowWidget::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
{
	CategoryName = InArgs._CategoryName;
	BackgroundBrush = InArgs._BackgroundBrush;
	ParentPanel = InArgs._ParentPanel;
	HoveredCategory = InArgs._HoveredCategory;

	STableRow<TSharedPtr<FName>>::Construct(
		STableRow<TSharedPtr<FName>>::FArguments(),
		InOwnerTable
	);

	// 호버 상태에 따라 색상 변경
	bool bIsHovered = (HoveredCategory.IsValid() && CategoryName.IsValid() && *HoveredCategory == *CategoryName);
	FLinearColor BorderColor = bIsHovered ? FLinearColor::White : FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);
	FLinearColor ImageTint = bIsHovered ? FLinearColor::White : FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	SetContent(
		SNew(SBorder)
		.BorderImage(BackgroundBrush)
		.BorderBackgroundColor(BorderColor)
		.Padding(10)
		[
			SNew(SBox)
				.HeightOverride(120)
				[
					SNew(SOverlay)

						// 배경 이미지 (호버 시 채도 복원)
						+ SOverlay::Slot()
						[
							SNew(SImage)
								.Image(BackgroundBrush)
								.ColorAndOpacity(ImageTint)
						]

						// 어두운 오버레이 (호버 시 투명도 감소)
						+ SOverlay::Slot()
						[
							SNew(SImage)
								.Image(new FSlateColorBrush(FLinearColor(0, 0, 0, bIsHovered ? 0.2f : 0.5f)))
						]

						// 카테고리 이름
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(FText::FromName(CategoryName.IsValid() ? *CategoryName : NAME_None))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", bIsHovered ? 20 : 18))
								.ColorAndOpacity(FLinearColor::White)
								.Justification(ETextJustify::Center)
								.ShadowOffset(FVector2D(2, 2))
								.ShadowColorAndOpacity(FLinearColor::Black)
						]
				]
		]
	);
}

void SFXCategoryRowWidget::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	STableRow<TSharedPtr<FName>>::OnMouseEnter(MyGeometry, MouseEvent);

	if (ParentPanel && CategoryName.IsValid())
	{
		ParentPanel->OnCategoryHovered(CategoryName);
	}
}

void SFXCategoryRowWidget::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	STableRow<TSharedPtr<FName>>::OnMouseLeave(MouseEvent);

	if (ParentPanel)
	{
		ParentPanel->OnCategoryUnhovered();
	}
}

