// Fill out your copyright notice in the Description page of Project Settings.

#include "FXAssetReferenceCollector.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Modules/ModuleManager.h"

TArray<FReferencedAsset> FFXAssetReferenceCollector::CollectAllReferences(const FSoftObjectPath& AssetPath)
{
	TArray<FReferencedAsset> ReferencedAssets;

	// Asset Registry 모듈 가져오기
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Asset Registry가 준비되었는지 확인
	if (AssetRegistry.IsLoadingAssets())
	{
		AssetRegistry.WaitForCompletion();
	}

	// FAssetData를 먼저 가져오기
	FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(*AssetPath.ToString());
	if (!AssetData.IsValid())
	{
		//UE_LOG(LogTemp, Warning, TEXT("[FX Reference Collector] Failed to get AssetData for: %s"), *AssetPath.ToString());
		return ReferencedAssets;
	}

	/*UE_LOG(LogTemp, Warning, TEXT("[FX Reference Collector] Found AssetData: PackageName=%s, AssetName=%s"),
		*AssetData.PackageName.ToString(), *AssetData.AssetName.ToString());*/

	// 방법 1: FName을 직접 사용하는 오버로드 시도
	FName PackageFName = AssetData.PackageName;

	// 필터링 헬퍼 함수 (로컬 람다)
	auto ShouldIncludeAsset = [](const FString& DependencyPath) -> bool
     {
         // /Game으로 시작하는 것만 포함 (게임 프로젝트 에셋)
         // /Niagara, /Script 등 엔진 기본 에셋은 제외
         return DependencyPath.StartsWith(TEXT("/Game/"));
     };
    // Hard 의존성 수집 - FName 오버로드 사용
    TArray<FName> HardDependencyNames;
    UE::AssetRegistry::FDependencyQuery HardQuery(UE::AssetRegistry::EDependencyQuery::Hard);

    bool bSuccess = AssetRegistry.GetDependencies(PackageFName, HardDependencyNames, UE::AssetRegistry::EDependencyCategory::Package, HardQuery);

    if (bSuccess)
    {
        for (const FName& DependencyName : HardDependencyNames)
        {
            FString DependencyPath = DependencyName.ToString();

            // /Game 하위 에셋만 포함
            if (!ShouldIncludeAsset(DependencyPath))
            {
                continue;
            }

            // Package 이름에서 Asset 이름 추출 시도
            FString PackagePath = DependencyPath;
            FString AssetName;
            
            // Package 경로에서 마지막 부분을 Asset 이름으로 사용
            int32 LastSlashIndex = INDEX_NONE;
            if (PackagePath.FindLastChar(TEXT('/'), LastSlashIndex))
            {
                AssetName = PackagePath.Mid(LastSlashIndex + 1);
            }
            else
            {
                AssetName = PackagePath;
            }

            // 전체 경로 구성 (Package.Asset 형식)
            FString FullAssetPath = PackagePath + TEXT(".") + AssetName;
            FSoftObjectPath RefPath(FullAssetPath);

            // Asset Registry에서 에셋 타입 가져오기
            FAssetData RefAssetData = AssetRegistry.GetAssetByObjectPath(*FullAssetPath);
            
            // 만약 찾지 못하면 Package 경로로 시도
            if (!RefAssetData.IsValid())
            {
                // Package 내의 모든 에셋 찾기
                TArray<FAssetData> AssetsInPackage;
                AssetRegistry.GetAssetsByPath(*PackagePath, AssetsInPackage, true);
                
                if (AssetsInPackage.Num() > 0)
                {
                    // 첫 번째 에셋 사용
                    RefAssetData = AssetsInPackage[0];
                }
            }

            FString AssetType;

            // AssetClassPath가 유효하면 사용, 아니면 AssetClass 사용
            if (RefAssetData.IsValid() && RefAssetData.AssetClassPath.IsValid())
            {
                AssetType = RefAssetData.AssetClassPath.GetAssetName().ToString();
            }
            else if (RefAssetData.IsValid() && RefAssetData.AssetClass != NAME_None)
            {
                AssetType = RefAssetData.AssetClass.ToString();
            }
            else
            {
                // AssetType이 비어있으면 에셋을 로드해서 확인
                UObject* LoadedAsset = RefPath.TryLoad();
                if (LoadedAsset)
                {
                    AssetType = LoadedAsset->GetClass()->GetName();
                }
                else
                {
                    AssetType = TEXT("Unknown");
                }
            }

            ReferencedAssets.Add(FReferencedAsset(RefPath, AssetType));// , TEXT("Hard")));
            //UE_LOG(LogTemp, Warning, TEXT("[FX Reference Hard] %s ### %s"), *RefPath.ToString(), *AssetType);
        }
    }

	return ReferencedAssets;
}

FSoftObjectPath FFXAssetReferenceCollector::AssetIdentifierToSoftObjectPath(const FAssetIdentifier& AssetId)
{
	// FAssetIdentifier는 PackageName을 포함하므로 이를 FSoftObjectPath로 변환
	FString PackageName = AssetId.PackageName.ToString();
	
	// PackageName에서 /Game/ 같은 경로를 추출하고, AssetName을 추가
	FString FullPath = PackageName;
	if (AssetId.ObjectName != NAME_None)
	{
		FString AssetName = AssetId.ObjectName.ToString();
		FullPath += TEXT(".");
		FullPath += AssetName;
	}

	return FSoftObjectPath(FullPath);
}
