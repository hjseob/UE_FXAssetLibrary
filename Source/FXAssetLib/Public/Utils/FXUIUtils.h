// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetThumbnail.h"

/**
 * UI 관련 유틸리티 함수들
 */
class FXASSETLIB_API FFXUIUtils
{
public:
	/**
	 * 썸네일 풀 생성 및 관리
	 */
	static TSharedPtr<FAssetThumbnailPool> CreateThumbnailPool(int32 PoolSize = 128);

	/**
	 * 에셋 썸네일 생성
	 */
	static TSharedPtr<FAssetThumbnail> CreateAssetThumbnail(
		const FAssetData& AssetData,
		int32 Width,
		int32 Height,
		TSharedPtr<FAssetThumbnailPool> ThumbnailPool
	);
};

