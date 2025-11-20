// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * FX Asset Library 전역 상수 정의
 */
namespace FFXAssetLibConstants
{
	// 탭 및 패널 이름
	static const FName FXLibraryTabName = TEXT("FX Library");
	
	// 기본 경로
	static const FString DefaultRootPath = TEXT("/Game/FXLib/");
	static const FString DefaultCategoryName = TEXT("Default");
	
	// UI 크기
	static const FVector2D RegistrationWindowSize(500.0f, 400.0f);
	static const FVector2D CategoryTileSize(200.0f, 120.0f);
	static const FVector2D AssetTileSize(120.0f, 130.0f);
	static const FVector2D ThumbnailSize(90.0f, 90.0f);
	
	// 카메라 스폰 거리
	static const float NiagaraSpawnDistance = 500.0f;
	
	// 썸네일 풀 크기
	static const int32 ThumbnailPoolSize = 128;
}

