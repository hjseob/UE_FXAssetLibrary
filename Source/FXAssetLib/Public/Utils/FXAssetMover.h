// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "AssetRegistry/AssetData.h"

// Forward declarations
struct FReferencedAsset;

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

	/**
	 * 에셋과 모든 Hard 참조를 재귀적으로 복사하고 참조를 업데이트
	 * @param SourceAssetPath 원본 에셋 경로
	 * @param DestinationFolderPath 대상 폴더 경로
	 * @param NewAssetName 새로운 에셋 이름 (확장자 제외)
	 * @param RootPath 루트 경로 (참조된 에셋들의 폴더 경로 생성에 사용)
	 * @return 복사된 에셋의 경로 (실패 시 빈 경로)
	 */
	static FSoftObjectPath CopyAssetWithReferences(
		const FSoftObjectPath& SourceAssetPath,
		const FString& DestinationFolderPath,
		const FString& NewAssetName,
		const FString& RootPath
	);

	/**
	 * 복사된 에셋의 참조를 새 경로로 업데이트
	 * @param AssetPath 업데이트할 에셋 경로
	 * @param ReferenceMap 원본 경로 -> 새 경로 매핑
	 * @return 성공 여부
	 */
	static bool UpdateAssetReferences(
		const FSoftObjectPath& AssetPath,
		const TMap<FSoftObjectPath, FSoftObjectPath>& ReferenceMap
	);

private:
	/**
	 * 대상 폴더에서 같은 이름의 에셋을 검색
	 * @param DestinationFolder 대상 폴더 경로
	 * @param AssetName 에셋 이름
	 * @return 찾은 에셋 경로 (없으면 빈 경로)
	 */
	static FSoftObjectPath FindExistingAsset(
		const FString& DestinationFolder,
		const FString& AssetName
	);

	/**
	 * 두 에셋이 같은 원본에서 복사된 것인지 확인
	 * @param SourceAssetPath 원본 에셋 경로
	 * @param ExistingAssetPath 기존 에셋 경로
	 * @return 같은 원본이면 true
	 */
	static bool CheckIfSameSourceAsset(
		const FSoftObjectPath& SourceAssetPath,
		const FSoftObjectPath& ExistingAssetPath
	);

	/**
	 * 재귀적으로 참조된 에셋을 복사하는 헬퍼 함수
	 * @param ReferencedAssets 복사할 참조된 에셋 목록
	 * @param RootPath 루트 경로
	 * @param ProcessedAssets 이미 처리된 에셋 경로 (중복 방지)
	 * @param ReferenceMap 원본 경로 -> 새 경로 매핑 (출력)
	 */
	static void CopyReferencedAssetsRecursive(
		const TArray<FReferencedAsset>& ReferencedAssets,
		const FString& RootPath,
		TSet<FSoftObjectPath>& ProcessedAssets,
		TMap<FSoftObjectPath, FSoftObjectPath>& ReferenceMap
	);

	/**
	 * 나이아가라 에셋의 내부 참조를 업데이트하는 전용 함수
	 * @param NiagaraSystem 업데이트할 나이아가라 시스템
	 * @param ReferenceMap 원본 경로 -> 새 경로 매핑
	 * @return 성공 여부
	 */
	static bool UpdateNiagaraAssetReferences(
		class UNiagaraSystem* NiagaraSystem,
		const TMap<FSoftObjectPath, FSoftObjectPath>& ReferenceMap
	);

	/**
	 * 머테리얼 에셋의 내부 참조를 업데이트하는 전용 함수
	 * Material Expression들을 순회하여 Material Function, Texture 등 참조 업데이트
	 * @param MaterialInterface 업데이트할 머테리얼 인터페이스
	 * @param ReferenceMap 원본 경로 -> 새 경로 매핑
	 * @return 성공 여부
	 */
	static bool UpdateMaterialAssetReferences(
		class UMaterialInterface* MaterialInterface,
		const TMap<FSoftObjectPath, FSoftObjectPath>& ReferenceMap
	);
};

