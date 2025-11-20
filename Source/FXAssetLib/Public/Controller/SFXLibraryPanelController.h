// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"

// Forward declarations
class FFXLibraryState;
class FFXLibraryModel;

/**
 * FX Library Panel Controller
 * SFXLibraryPanel의 이벤트 핸들러를 처리하고 State를 업데이트
 */
class FXASSETLIB_API SFXLibraryPanelController
{
public:
	SFXLibraryPanelController();
	~SFXLibraryPanelController();

	// 초기화
	void Initialize(
		TSharedPtr<FFXLibraryState> InState,
		TSharedPtr<FFXLibraryModel> InModel
	);

	// 버튼 클릭 이벤트
	FReply OnRefreshClicked();
	FReply OnResetNiagaraClicked();
	FReply OnCleanupClicked();

	// 에셋 관련 이벤트
	void SpawnNiagaraActor(TSharedPtr<FSoftObjectPath> AssetPath);
	void BrowseToAsset(TSharedPtr<FSoftObjectPath> AssetPath);
	void RemoveAssetFromLibrary(TSharedPtr<FSoftObjectPath> AssetPath);

	// 카테고리 관련 이벤트
	void OnCategoryChanged(TSharedPtr<FName> Category);
	void OnCategoryHovered(TSharedPtr<FName> Category);
	void OnCategoryUnhovered();

	// State 접근
	TSharedPtr<FFXLibraryState> GetState() const { return State; }
	TSharedPtr<FFXLibraryModel> GetModel() const { return Model; }

private:
	TSharedPtr<FFXLibraryState> State;
	TSharedPtr<FFXLibraryModel> Model;
};

