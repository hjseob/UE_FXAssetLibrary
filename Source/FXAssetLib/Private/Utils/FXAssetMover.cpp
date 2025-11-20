// Fill out your copyright notice in the Description page of Project Settings.

#include "Utils/FXAssetMover.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "Editor.h"
#include "ObjectTools.h"
#include "Utils/FXAssetOrganizer.h"

FSoftObjectPath FXAssetMover::CopyAssetWithNewName(
	const FSoftObjectPath& SourceAssetPath,
	const FString& DestinationFolderPath,
	const FString& NewAssetName)
{
	// AssetTools 모듈 가져오기
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	IAssetTools& AssetTools = AssetToolsModule.Get();

	// 원본 에셋 로드
	UObject* SourceAsset = SourceAssetPath.TryLoad();
	if (!SourceAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load source asset: %s"), *SourceAssetPath.ToString());
		return FSoftObjectPath();
	}

	// 대상 경로 생성
	FString DestinationPath = DestinationFolderPath;
	if (!DestinationPath.EndsWith(TEXT("/")))
	{
		DestinationPath += TEXT("/");
	}
	DestinationPath += NewAssetName;

	// 폴더 생성
	if (!FXAssetOrganizer::CreateFolderPath(DestinationFolderPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create folder: %s"), *DestinationFolderPath);
		return FSoftObjectPath();
	}

	// 에셋 복사 (DuplicateAsset 사용)
	UObject* DuplicatedAsset = AssetTools.DuplicateAsset(NewAssetName, DestinationFolderPath, SourceAsset);
	if (DuplicatedAsset)
	{
		FSoftObjectPath CopiedPath(DuplicatedAsset);
		UE_LOG(LogTemp, Log, TEXT("Successfully copied asset: %s -> %s"), 
			*SourceAssetPath.ToString(), *CopiedPath.ToString());
		return CopiedPath;
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to copy asset: %s to %s"), *SourceAssetPath.ToString(), *DestinationPath);
	return FSoftObjectPath();
}

TArray<FSoftObjectPath> FXAssetMover::CopyAssets(
	const TArray<FAssetData>& SourceAssets,
	const FString& DestinationFolderPath,
	const FString& BaseAssetName)
{
	TArray<FSoftObjectPath> CopiedAssets;

	for (int32 i = 0; i < SourceAssets.Num(); ++i)
	{
		FString NewName = BaseAssetName;
		if (SourceAssets.Num() > 1)
		{
			NewName += FString::Printf(TEXT("_%d"), i);
		}

		FSoftObjectPath SourcePath = SourceAssets[i].ToSoftObjectPath();
		FSoftObjectPath CopiedPath = CopyAssetWithNewName(SourcePath, DestinationFolderPath, NewName);
		
		if (CopiedPath.IsValid())
		{
			CopiedAssets.Add(CopiedPath);
		}
	}

	return CopiedAssets;
}

