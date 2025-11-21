// Fill out your copyright notice in the Description page of Project Settings.

#include "Utils/FXAssetOrganizer.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"

bool FXAssetOrganizer::CreateFolderPath(const FString& FolderPath)
{
	if (FolderPath.IsEmpty())
	{
		return false;
	}

	// 경로를 정규화 (이중 슬래시 제거)
	FString NormalizedPath = FolderPath;
	NormalizedPath.ReplaceInline(TEXT("\\"), TEXT("/"));
	
	// 이중 슬래시를 단일 슬래시로 변환
	while (NormalizedPath.Contains(TEXT("//")))
	{
		NormalizedPath.ReplaceInline(TEXT("//"), TEXT("/"));
	}
	
	// 경로가 /Game/으로 시작하지 않으면 /Game/ 추가
	if (!NormalizedPath.StartsWith(TEXT("/Game/")))
	{
		if (NormalizedPath.StartsWith(TEXT("/")))
		{
			NormalizedPath = TEXT("/Game") + NormalizedPath;
		}
		else
		{
			NormalizedPath = TEXT("/Game/") + NormalizedPath;
		}
	}

	// Content 경로를 물리적 파일 시스템 경로로 변환
	FString ContentDir = FPaths::ProjectContentDir();
	
	// /Game/을 Content 디렉토리 경로로 변환
	FString PhysicalPath = NormalizedPath;
	PhysicalPath.ReplaceInline(TEXT("/Game/"), *ContentDir);
	
	// 슬래시를 플랫폼별 구분자로 정규화
	FPaths::NormalizeDirectoryName(PhysicalPath);

	// IPlatformFile을 사용하여 폴더 생성 (CreateDirectoryTree는 재귀적으로 모든 경로 생성)
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	
	// 폴더가 이미 존재하는지 확인
	if (PlatformFile.DirectoryExists(*PhysicalPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Folder already exists: %s"), *NormalizedPath);
		return true;
	}
	
	// 재귀적으로 폴더 생성
	if (!PlatformFile.CreateDirectoryTree(*PhysicalPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create directory tree: %s (Physical: %s)"), 
			*NormalizedPath, *PhysicalPath);
		return false;
	}

	// Asset Registry 갱신 (Content Browser에 반영)
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().ScanPathsSynchronous({ NormalizedPath }, true);

	UE_LOG(LogTemp, Log, TEXT("Successfully created folder: %s"), *NormalizedPath);
	return true;
}

FString FXAssetOrganizer::GetFolderPathForAssetType(const FString& RootPath, const FString& CategoryName, const FString& AssetType)
{
	// RootPath 정규화 (끝의 슬래시 제거)
	FString NormalizedRoot = RootPath;
	NormalizedRoot.ReplaceInline(TEXT("\\"), TEXT("/"));
	while (NormalizedRoot.EndsWith(TEXT("/")))
	{
		NormalizedRoot.RemoveAt(NormalizedRoot.Len() - 1);
	}
	
	FString FolderName = AssetTypeToFolderName(AssetType);
	
	// NiagaraSystem인 경우 카테고리별로 분류
	if (AssetType == "NiagaraSystem")
	{
		return FString::Printf(TEXT("%s/Particles/%s"), *NormalizedRoot, *CategoryName);
	}
	
	// 다른 타입은 공유 폴더에
	return FString::Printf(TEXT("%s/%s"), *NormalizedRoot, *FolderName);
}

FString FXAssetOrganizer::AssetTypeToFolderName(const FString& AssetType)
{
	// 에셋 타입을 폴더 이름으로 매핑 (파스칼 케이스)
	// Material과 MaterialInstance는 Materials 폴더로
	if (AssetType == "Material" || AssetType == "MaterialInstanceConstant")
	{
		return TEXT("Materials");
	}
	// Texture는 Textures 폴더로
	else if (AssetType == "Texture2D" || AssetType == "Texture")
	{
		return TEXT("Textures");
	}
	// Mesh 타입은 Meshes 폴더로 (향후 확장용)
	else if (AssetType == "StaticMesh" || AssetType == "SkeletalMesh")
	{
		return TEXT("Meshes");
	}
	// NiagaraSystem은 Particles 폴더로 (기존 유지)
	else if (AssetType == "NiagaraSystem")
	{
		return TEXT("Particles");
	}
	// VectorFields (향후 확장용)
	else if (AssetType == "VectorField" || AssetType == "VectorFieldStatic")
	{
		return TEXT("VectorFields");
	}
	// NiagaraScripts (향후 확장용)
	else if (AssetType == "NiagaraScript")
	{
		return TEXT("NiagaraScripts");
	}
	
	// 기본값: 에셋 타입 이름 그대로 사용
	return AssetType;
}

