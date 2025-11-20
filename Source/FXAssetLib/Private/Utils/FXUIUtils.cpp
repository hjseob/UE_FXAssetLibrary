// Fill out your copyright notice in the Description page of Project Settings.

#include "Utils/FXUIUtils.h"
#include "AssetThumbnail.h"
#include "Core/FXAssetLibConstants.h"

TSharedPtr<FAssetThumbnailPool> FFXUIUtils::CreateThumbnailPool(int32 PoolSize)
{
	return MakeShared<FAssetThumbnailPool>(PoolSize);
}

TSharedPtr<FAssetThumbnail> FFXUIUtils::CreateAssetThumbnail(
	const FAssetData& AssetData,
	int32 Width,
	int32 Height,
	TSharedPtr<FAssetThumbnailPool> ThumbnailPool)
{
	if (!ThumbnailPool.IsValid())
	{
		ThumbnailPool = CreateThumbnailPool();
	}

	return MakeShared<FAssetThumbnail>(AssetData, Width, Height, ThumbnailPool);
}

