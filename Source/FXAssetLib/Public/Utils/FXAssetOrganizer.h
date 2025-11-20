// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"

/**
 * 에셋 폴더 구조 관리 클래스
 */
class FXASSETLIB_API FXAssetOrganizer
{
public:
	/**
	 * 폴더 경로 생성 (존재하지 않으면 생성)
	 * @param FolderPath 생성할 폴더 경로 (예: "/Game/FXLib/Particles/Fire")
	 * @return 성공 여부
	 */
	static bool CreateFolderPath(const FString& FolderPath);

	/**
	 * 에셋 타입에 따른 폴더 경로 생성
	 * @param RootPath 루트 경로 (예: "/Game/FXLib")
	 * @param CategoryName 카테고리 이름 (예: "Fire")
	 * @param AssetType 에셋 타입 (예: "NiagaraSystem", "Material", "Texture2D")
	 * @return 생성된 폴더 경로
	 */
	static FString GetFolderPathForAssetType(const FString& RootPath, const FString& CategoryName, const FString& AssetType);

private:
	/**
	 * 에셋 타입을 폴더 이름으로 변환
	 */
	static FString AssetTypeToFolderName(const FString& AssetType);
};

