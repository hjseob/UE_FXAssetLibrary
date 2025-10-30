// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FXLibrarySettings.generated.h"


/**
 * 카테고리 정보 구조체
 * 카테고리 이름, 아이콘, 등록된 에셋들을 함께 관리
 */
USTRUCT(BlueprintType)
struct FFXCategoryData
{
    GENERATED_BODY()

    // 카테고리 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Category")
    FName CategoryName;

    // 카테고리 아이콘 (Texture2D)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Category", meta = (AllowedClasses = "/Script/Engine.Texture2D"))
    FSoftObjectPath IconPath;

    // 등록된 나이아가라 에셋 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<FSoftObjectPath> Assets;

    // 기본 생성자
    FFXCategoryData()
        : CategoryName(NAME_None)
        , IconPath()
        , Assets()
    {
    }

    // 이름으로 초기화
    FFXCategoryData(FName InName)
        : CategoryName(InName)
        , IconPath()
        , Assets()
    {
    }
};


/**
* FX Library Settings
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "FX Library Settings"))
class FXASSETLIB_API UFXLibrarySettings : public UDeveloperSettings
{
    GENERATED_BODY()
public:
	UFXLibrarySettings();
	~UFXLibrarySettings();

    // ⭐ 추가!
    virtual void PostInitProperties() override;

    
    UPROPERTY(Config, EditAnywhere, Category = "FX Library")
    TArray<FFXCategoryData> Categories;

    FFXCategoryData* FindCategory(FName CategoryName);
    const FFXCategoryData* FindCategory(FName CategoryName) const;

    FFXCategoryData* AddCategory(FName CategoryName, const FSoftObjectPath& IconPath = FSoftObjectPath());
    bool AddAssetToCategory(FName CategoryName, const FSoftObjectPath& AssetPath);
    int32 RemoveAssetFromCategory(FName CategoryName, const FSoftObjectPath& AssetPath);




 //   // 카테고리 → 등록된 나이아가라 에셋 경로
 //   UPROPERTY(Config, EditAnywhere, Category = "FX Library")
 //   TMap<FName, FFXAssetArray> CategoryToAssets;
 //   //TMap<FName, TArray<FSoftObjectPath>> CategoryToAssets;
 //   // 정확한 에러가 나왔습니다! Unreal Header Tool(UHT)이 TMap의 값으로 TArray<FSoftObjectPath>를 허용하지 않습니다.
 //   // 문제: UHT는 UPROPERTY로 노출되는 TMap의 값 타입에 대해 제약이 있습니다.TArray는 TMap의 값으로 직접 사용할 수 없습니다.
	//// TArray를 감싸는 구조체로 변경
 //   
 //   // 카테고리 → 커스텀 아이콘 경로 (추가!)
 //   UPROPERTY(Config, EditAnywhere, Category = "FX Library | Icons", meta = (AllowedClasses = "/Script/Engine.Texture2D"))
 //   TMap<FName, FSoftObjectPath> CategoryIcons;



    virtual FName GetCategoryName() const override;
    virtual FText GetSectionText() const override;
};
