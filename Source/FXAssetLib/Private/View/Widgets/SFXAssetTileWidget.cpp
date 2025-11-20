// Fill out your copyright notice in the Description page of Project Settings.

#include "View/Widgets/SFXAssetTileWidget.h"
#include "View/SFXLibraryPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
//#include "Widgets/Layout/SVerticalBox.h"
#include "Widgets/Text/STextBlock.h"
#include "AssetThumbnail.h"
#include "Styling/CoreStyle.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

void SFXAssetTileWidget::Construct(const FArguments& InArgs)
{
	AssetPath = InArgs._AssetPath;
	ParentPanel = InArgs._ParentPanel;
	Thumbnail = InArgs._Thumbnail;
	AssetName = InArgs._AssetName;

	ChildSlot
		[
			SNew(SBorder)
				// 호버 시 배경 색상 변경
				.BorderBackgroundColor_Lambda([this]() -> FLinearColor
					{
						return IsHovered() ? FLinearColor(0.7f, 0.5f, 0.4f, 1.0f) : FLinearColor(0.2f, 0.2f, 0.3f, 1.0f);
					})
				.Padding(5)
				[
					SNew(SVerticalBox)
						// 썸네일 이미지
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(1)
						[
							SNew(SBox)
								.WidthOverride(90)
								.HeightOverride(90)
								[
									Thumbnail.IsValid() ? Thumbnail->MakeThumbnailWidget() : SNew(SBox)
								]
						]

						// 에셋 이름
						+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.MaxHeight(40)
							.Padding(2, 4, 2, 2)
							[
								SNew(STextBlock)
									.Text(FText::FromString(AssetName))
									.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
									.Justification(ETextJustify::Left)
									.AutoWrapText(true)
									.ToolTipText(FText::FromString(AssetName))
							]
				]
		];
}

FReply SFXAssetTileWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		// 컨텍스트 메뉴 생성
		FMenuBuilder MenuBuilder(true, nullptr);

		// Browse 메뉴 항목
		MenuBuilder.AddMenuEntry(
			FText::FromString(TEXT("Find in Content Browser")),
			FText::FromString(TEXT("Navigate to this asset in the Content Browser")),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "SystemWideCommands.FindInContentBrowser"),
			FUIAction(FExecuteAction::CreateLambda([this]()
				{
					if (ParentPanel && AssetPath.IsValid())
					{
						ParentPanel->OnBrowseToAsset(AssetPath);
					}
				}))
		);

		// 구분선
		MenuBuilder.AddMenuSeparator();

		// Remove 메뉴 항목
		MenuBuilder.AddMenuEntry(
			FText::FromString(TEXT("Remove from Library")),
			FText::FromString(TEXT("Remove this asset from the FX Library")),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"),
			FUIAction(FExecuteAction::CreateLambda([this]()
				{
					if (ParentPanel && AssetPath.IsValid())
					{
						ParentPanel->OnRemoveAssetFromLibrary(AssetPath);
					}
				}))
		);

		// 메뉴 표시
		FSlateApplication::Get().PushMenu(
			AsShared(),
			FWidgetPath(),
			MenuBuilder.MakeWidget(),
			MouseEvent.GetScreenSpacePosition(),
			FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
		);

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

