// Fill out your copyright notice in the Description page of Project Settings.


#include "FXLibrarySettings.h"


UFXLibrarySettings::UFXLibrarySettings()
{
}
UFXLibrarySettings::~UFXLibrarySettings()
{
}

void UFXLibrarySettings::PostInitProperties()
{
    Super::PostInitProperties();

    // Config 로드 후 실행됨
    // 카테고리가 비어있으면 기본값 설정
    if (Categories.Num() == 0)
    {
        //FFXCategoryData FireCategory(TEXT("Fire"));
        //Categories.Add(FireCategory);

        //FFXCategoryData WaterCategory(TEXT("Water"));
        //Categories.Add(WaterCategory);

        //FFXCategoryData SmokeCategory(TEXT("Smoke"));
        //Categories.Add(SmokeCategory);

        //FFXCategoryData ElectricCategory(TEXT("Electric"));
        //Categories.Add(ElectricCategory);


        
        FFXCategoryData FireCategory(TEXT("Fire"));
        FireCategory.IconPath = FSoftObjectPath(TEXT("/Game/UI/Icons/Fire.Fire"));
        Categories.Add(FireCategory);

        FFXCategoryData WaterCategory(TEXT("Water"));
        WaterCategory.IconPath = FSoftObjectPath(TEXT("/Game/UI/Icons/Water.Water"));
        Categories.Add(WaterCategory);

        FFXCategoryData SmokeCategory(TEXT("Smoke"));
        SmokeCategory.IconPath = FSoftObjectPath(TEXT("/Game/UI/Icons/Smoke.Smoke"));
        Categories.Add(SmokeCategory);

        FFXCategoryData ElectricCategory(TEXT("Electric"));
        ElectricCategory.IconPath = FSoftObjectPath(TEXT("/Game/UI/Icons/Electric.Electric"));
        Categories.Add(ElectricCategory);


        // 기본값 저장
        SaveConfig();
    }
}



FName UFXLibrarySettings::GetCategoryName() const
{
    return TEXT("Plugins");
}

FText UFXLibrarySettings::GetSectionText() const
{
    return NSLOCTEXT("FXLib", "Section", "FX Library");
}

FFXCategoryData* UFXLibrarySettings::FindCategory(FName InCategoryName)
{
    for (FFXCategoryData& Cat : Categories)
    {
        if (Cat.CategoryName == InCategoryName)
        {
            return &Cat;
        }
    }
    return nullptr;
}

const FFXCategoryData* UFXLibrarySettings::FindCategory(FName InCategoryName) const
{
    for (const FFXCategoryData& Cat : Categories)
    {
        if (Cat.CategoryName == InCategoryName)
        {
            return &Cat;
        }
    }
    return nullptr;
}

FFXCategoryData* UFXLibrarySettings::AddCategory(FName InCategoryName, const FSoftObjectPath& IconPath)
{
    // 이미 존재하는지 확인
    if (FFXCategoryData* Existing = FindCategory(InCategoryName))
    {
        return Existing;
    }

    // 새 카테고리 추가
    FFXCategoryData NewCategory(InCategoryName);
    NewCategory.IconPath = IconPath;

    Categories.Add(NewCategory);
    SaveConfig();

    return &Categories.Last();
}

bool UFXLibrarySettings::AddAssetToCategory(FName InCategoryName, const FSoftObjectPath& AssetPath)
{
    FFXCategoryData* Category = FindCategory(InCategoryName);
    if (!Category)
    {
        return false;
    }

    Category->Assets.AddUnique(AssetPath);
    SaveConfig();
    return true;
}

int32 UFXLibrarySettings::RemoveAssetFromCategory(FName InCategoryName, const FSoftObjectPath& AssetPath)
{
    FFXCategoryData* Category = FindCategory(InCategoryName);
    if (!Category)
    {
        return 0;
    }

    int32 Removed = Category->Assets.Remove(AssetPath);
    if (Removed > 0)
    {
        SaveConfig();
    }
    return Removed;
}

