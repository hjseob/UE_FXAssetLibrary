// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "AssetRegistry/AssetData.h"

/**
 * 에셋 복사/이동 유틸리티 클래스
 */
class FXASSETLIB_API FXAssetMover
{
public:
	/**
	 * 에셋을 새 경로로 복사하고 이름 변경
	 * @param SourceAssetPath 원본 에셋 경로
	 * @param DestinationFolderPath 대상 폴더 경로
	 * @param NewAssetName 새로운 에셋 이름 (확장자 제외)
	 * @return 복사된 에셋의 경로 (실패 시 빈 경로)
	 */
	static FSoftObjectPath CopyAssetWithNewName(
		const FSoftObjectPath& SourceAssetPath,
		const FString& DestinationFolderPath,
		const FString& NewAssetName
	);

	/**
	 * 여러 에셋을 복사
	 */
	static TArray<FSoftObjectPath> CopyAssets(
		const TArray<FAssetData>& SourceAssets,
		const FString& DestinationFolderPath,
		const FString& BaseAssetName
	);
};
