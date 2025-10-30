// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"       
#include "Widgets/Views/STileView.h"       
#include "Widgets/Views/STableViewBase.h"  
#include "Widgets/Views/STableRow.h"       




class FXASSETLIB_API SFXLibraryPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFXLibraryPanel)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);


private:
	void LoadFromSettings();
	void SetCategory(FName Cat);
	void OnCatChanged(TSharedPtr<FName> NewSel, ESelectInfo::Type SelectInfo) const;

	// category and asset tile list 생성 콜백
	TSharedRef<ITableRow> GenCatRow(TSharedPtr<FName> Item, const TSharedRef<STableViewBase>& Owner);
	TSharedRef<ITableRow> GenAssetTile(TSharedPtr<FSoftObjectPath> Item, const TSharedRef<STableViewBase>& Owner);

	// 썸네일 풀 가져오기
	TSharedPtr<class FAssetThumbnailPool> GetThumbnailPool();



	// button click call backs
	void SpawnNiagaraActor(TSharedPtr<FSoftObjectPath> AssetPath);
	FReply OnRefreshClicked();
	FReply OnResetNiagaraClicked();
	void BrowseToAsset(TSharedPtr<FSoftObjectPath> AssetPath);
	void RemoveAssetFromLibrary(TSharedPtr<FSoftObjectPath> AssetPath);



private:
	TMap<FName, TArray<FSoftObjectPath>> CatsToAssets;		// 카테고리 → 에셋 경로 매핑
	TMap<FName, FSoftObjectPath> CategoryIcons; // 추가!
	TArray<TSharedPtr<FName>> Categories;					// 카테고리 목록 (UI 표시용)
	mutable TArray<TSharedPtr<FSoftObjectPath>> Visible;	// 현재 선택된 카테고리의 에셋 목록

	TSharedPtr<SListView<TSharedPtr<FName>>> CategoryList;	// 카테고리 리스트 뷰
	mutable TSharedPtr<STileView<TSharedPtr<FSoftObjectPath>>> AssetTileView; // 에셋 리스트 뷰

	// 썸네일 풀 (에셋 미리보기 이미지 관리)
	TSharedPtr<class FAssetThumbnailPool> ThumbnailPool;


	//FReply OnAssetTileMouseButtonDown(const FGeometry& Geo, const FPointerEvent& MouseEvent, TSharedPtr<FSoftObjectPath> Item);

	// 현재 선택된 카테고리 (새로고침 후 복원용)
	TSharedPtr<FName> CurrentSelectedCategory;

	// 현재 호버 중인 카테고리 (nullptr이면 호버 없음)
	mutable TSharedPtr<FName> HoveredCategory;

};



/*
* 나중에 추가할 "카테고리 등록 UI" 예시 코드
* 
// 예시: 커스텀 에디터 창에서
UFXLibrarySettings* Settings = GetMutableDefault<UFXLibrarySettings>();

// 카테고리 추가 (이름 + 아이콘 동시에)
FFXCategoryData* NewCat = Settings->AddCategory(
	TEXT("Explosion"),
	FSoftObjectPath(TEXT("/Game/UI/Icons/Explosion_Icon.Explosion_Icon"))
);

// 에셋 추가
Settings->AddAssetToCategory(TEXT("Explosion"), SomeNiagaraPath);
*/


