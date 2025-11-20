// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"

// Forward declarations
struct FAssetIdentifier;

/**
 * 참조된 에셋 정보를 담는 구조체
 */
struct FXASSETLIB_API FReferencedAsset
{
	FSoftObjectPath AssetPath;      // 참조된 에셋 경로
	FString AssetType;               // 에셋 타입 (예: "Texture2D", "Material")
	//FString ReferenceType;           // 참조 타입 (Hard/Soft)

	FReferencedAsset()
		: AssetPath()
		, AssetType()
		//, ReferenceType()
	{
	}

	FReferencedAsset(const FSoftObjectPath& InAssetPath, const FString& InAssetType)// , const FString& InReferenceType)
		: AssetPath(InAssetPath)
		, AssetType(InAssetType)
		//, ReferenceType(InReferenceType)
	{
	}
};

/**
 * 에셋 참조 수집 유틸리티 클래스
 * Reference Viewer의 "Show Referenced" 기능과 유사한 기능 제공
 */
class FXASSETLIB_API FFXAssetReferenceCollector
{
public:
	/**
	 * 특정 에셋이 참조하는 모든 에셋 목록을 수집합니다.
	 * @param AssetPath 참조를 수집할 에셋의 경로
	 * @return 참조된 에셋 목록
	 */
	static TArray<FReferencedAsset> CollectAllReferences(const FSoftObjectPath& AssetPath);

private:
	/**
	 * FAssetIdentifier를 FSoftObjectPath로 변환
	 */
	static FSoftObjectPath AssetIdentifierToSoftObjectPath(const FAssetIdentifier& AssetId);
};

