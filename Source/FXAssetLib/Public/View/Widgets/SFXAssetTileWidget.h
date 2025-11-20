// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "UObject/SoftObjectPath.h"

// Forward declarations
class SFXLibraryPanel;
class FAssetThumbnail;

/**
 * 에셋 타일 위젯
 * 에셋 리스트에서 각 에셋을 타일 형태로 표시하는 위젯
 */
class FXASSETLIB_API SFXAssetTileWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFXAssetTileWidget) {}
		SLATE_ARGUMENT(TSharedPtr<FSoftObjectPath>, AssetPath)
		SLATE_ARGUMENT(SFXLibraryPanel*, ParentPanel)
		SLATE_ARGUMENT(TSharedPtr<FAssetThumbnail>, Thumbnail)
		SLATE_ARGUMENT(FString, AssetName)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	TSharedPtr<FSoftObjectPath> AssetPath;
	SFXLibraryPanel* ParentPanel;
	TSharedPtr<FAssetThumbnail> Thumbnail;
	FString AssetName;
};

