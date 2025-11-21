// Fill out your copyright notice in the Description page of Project Settings.

#include "Utils/FXAssetMover.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "Editor.h"
#include "ObjectTools.h"
#include "Utils/FXAssetOrganizer.h"
#include "Utils/FXAssetReferenceCollector.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Engine/Engine.h"
#include "Serialization/ArchiveReplaceObjectRef.h"
#include "UObject/SavePackage.h"
#include "UObject/Class.h"
#include "UObject/PropertyIterator.h"
#include "UObject/UnrealType.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "NiagaraRendererProperties.h"
#include "NiagaraSpriteRendererProperties.h"
#include "NiagaraMeshRendererProperties.h"
#include "NiagaraRibbonRendererProperties.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionTextureSampleParameter.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialFunctionInterface.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"

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

FSoftObjectPath FXAssetMover::CopyAssetWithReferences(
	const FSoftObjectPath& SourceAssetPath,
	const FString& DestinationFolderPath,
	const FString& NewAssetName,
	const FString& RootPath)
{
	// 1. 먼저 참조된 에셋들을 수집
	TArray<FReferencedAsset> ReferencedAssets = FFXAssetReferenceCollector::CollectAllReferences(SourceAssetPath);
	
	// 2. 참조된 에셋들을 재귀적으로 복사
	TSet<FSoftObjectPath> ProcessedAssets;
	TMap<FSoftObjectPath, FSoftObjectPath> ReferenceMap;
	
	CopyReferencedAssetsRecursive(ReferencedAssets, RootPath, ProcessedAssets, ReferenceMap);
	
	// 3. 메인 에셋 복사
	FSoftObjectPath CopiedAssetPath = CopyAssetWithNewName(SourceAssetPath, DestinationFolderPath, NewAssetName);
	
	if (!CopiedAssetPath.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to copy main asset: %s"), *SourceAssetPath.ToString());
		return FSoftObjectPath();
	}
	
	// 4. 복사된 메인 에셋의 참조 업데이트
	if (!UpdateAssetReferences(CopiedAssetPath, ReferenceMap))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to update references for copied asset: %s"), *CopiedAssetPath.ToString());
	}
	
	return CopiedAssetPath;
}

FSoftObjectPath FXAssetMover::FindExistingAsset(
	const FString& DestinationFolder,
	const FString& AssetName)
{
	// Asset Registry 모듈 가져오기
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// 대상 폴더의 모든 에셋 검색
	TArray<FAssetData> AssetsInFolder;
	AssetRegistry.GetAssetsByPath(*DestinationFolder, AssetsInFolder, true);

	// 같은 이름의 에셋 찾기
	for (const FAssetData& AssetData : AssetsInFolder)
	{
		if (AssetData.AssetName.ToString() == AssetName)
		{
			return AssetData.ToSoftObjectPath();
		}
	}

	return FSoftObjectPath();
}

bool FXAssetMover::CheckIfSameSourceAsset(
	const FSoftObjectPath& SourceAssetPath,
	const FSoftObjectPath& ExistingAssetPath)
{
	// Asset Registry 모듈 가져오기
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// 두 에셋의 FAssetData 가져오기
	FAssetData SourceAssetData = AssetRegistry.GetAssetByObjectPath(*SourceAssetPath.ToString());
	FAssetData ExistingAssetData = AssetRegistry.GetAssetByObjectPath(*ExistingAssetPath.ToString());

	if (!SourceAssetData.IsValid() || !ExistingAssetData.IsValid())
	{
		return false;
	}

	// 원본 경로가 같으면 같은 에셋으로 간주
	// (실제로는 복사 시 원본 경로를 Asset UserData나 Tag로 저장하고 비교하는 것이 더 정확함)
	// 현재는 원본 경로가 같은지 확인
	if (SourceAssetPath == ExistingAssetPath)
	{
		return true;
	}

	// 원본 경로가 다르면 다른 에셋으로 간주
	// 하지만 실제로는 같은 원본에서 복사했을 수도 있으므로,
	// 더 정확한 방법은 복사 시 원본 경로를 메타데이터로 저장하고 비교하는 것
	// 여기서는 일단 false 반환 (다른 에셋으로 처리하여 넘버링 추가)
	return false;
}

void FXAssetMover::CopyReferencedAssetsRecursive(
	const TArray<FReferencedAsset>& ReferencedAssets,
	const FString& RootPath,
	TSet<FSoftObjectPath>& ProcessedAssets,
	TMap<FSoftObjectPath, FSoftObjectPath>& ReferenceMap)
{
	for (const FReferencedAsset& RefAsset : ReferencedAssets)
	{
		// 이미 처리된 에셋은 스킵
		if (ProcessedAssets.Contains(RefAsset.AssetPath))
		{
			continue;
		}
		
		ProcessedAssets.Add(RefAsset.AssetPath);
		
		// 에셋 타입에 따라 폴더 경로 결정
		FString DestinationFolder = FXAssetOrganizer::GetFolderPathForAssetType(RootPath, TEXT(""), RefAsset.AssetType);
		
		// 원본 에셋 이름 추출
		FString AssetName = RefAsset.AssetPath.GetAssetName();
		
		// 중복 체크: 같은 이름의 에셋이 이미 존재하는지 확인
		FSoftObjectPath ExistingAssetPath = FindExistingAsset(DestinationFolder, AssetName);
		FSoftObjectPath CopiedPath;
		
		if (ExistingAssetPath.IsValid())
		{
			// 같은 원본에서 복사한 에셋인지 확인
			if (CheckIfSameSourceAsset(RefAsset.AssetPath, ExistingAssetPath))
			{
				// 같은 에셋이면 기존 에셋 재사용
				CopiedPath = ExistingAssetPath;
				UE_LOG(LogTemp, Log, TEXT("Reusing existing asset (same source): %s -> %s (Type: %s)"), 
					*RefAsset.AssetPath.ToString(), *CopiedPath.ToString(), *RefAsset.AssetType);
			}
			else
			{
				// 다른 에셋이면 넘버링 추가
				int32 Counter = 1;
				FString NewAssetName;
				FSoftObjectPath TestPath;
				
				do
				{
					NewAssetName = FString::Printf(TEXT("%s_%02d"), *AssetName, Counter);
					TestPath = FindExistingAsset(DestinationFolder, NewAssetName);
					Counter++;
				} while (TestPath.IsValid() && Counter < 100); // 최대 99까지 시도
				
				// 새 이름으로 복사
				CopiedPath = CopyAssetWithNewName(RefAsset.AssetPath, DestinationFolder, NewAssetName);
				if (CopiedPath.IsValid())
				{
					UE_LOG(LogTemp, Log, TEXT("Copied referenced asset with numbering: %s -> %s (Type: %s)"), 
						*RefAsset.AssetPath.ToString(), *CopiedPath.ToString(), *RefAsset.AssetType);
				}
			}
		}
		else
		{
			// 기존 에셋이 없으면 그대로 복사
			CopiedPath = CopyAssetWithNewName(RefAsset.AssetPath, DestinationFolder, AssetName);
			if (CopiedPath.IsValid())
			{
				UE_LOG(LogTemp, Log, TEXT("Copied referenced asset: %s -> %s (Type: %s)"), 
					*RefAsset.AssetPath.ToString(), *CopiedPath.ToString(), *RefAsset.AssetType);
			}
		}
		
		if (CopiedPath.IsValid())
		{
			// 참조 맵에 추가
			ReferenceMap.Add(RefAsset.AssetPath, CopiedPath);
			
			// 복사된 에셋의 참조도 재귀적으로 처리
			TArray<FReferencedAsset> NestedReferences = FFXAssetReferenceCollector::CollectAllReferences(RefAsset.AssetPath);
			if (NestedReferences.Num() > 0)
			{
				CopyReferencedAssetsRecursive(NestedReferences, RootPath, ProcessedAssets, ReferenceMap);
			}
			
			// 복사된 에셋의 참조도 업데이트
			UpdateAssetReferences(CopiedPath, ReferenceMap);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to copy referenced asset: %s"), *RefAsset.AssetPath.ToString());
		}
	}
}

bool FXAssetMover::UpdateAssetReferences(
	const FSoftObjectPath& AssetPath,
	const TMap<FSoftObjectPath, FSoftObjectPath>& ReferenceMap)
{
	if (ReferenceMap.Num() == 0)
	{
		return true; // 업데이트할 참조가 없음
	}
	
	// 에셋 로드
	UObject* Asset = AssetPath.TryLoad();
	if (!Asset)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load asset for reference update: %s"), *AssetPath.ToString());
		return false;
	}
	
	// 나이아가라 에셋인 경우 전용 함수 호출
	if (UNiagaraSystem* NiagaraSystem = Cast<UNiagaraSystem>(Asset))
	{
		return UpdateNiagaraAssetReferences(NiagaraSystem, ReferenceMap);
	}
	
	// 머테리얼 에셋인 경우 전용 함수 호출
	if (UMaterialInterface* MaterialInterface = Cast<UMaterialInterface>(Asset))
	{
		return UpdateMaterialAssetReferences(MaterialInterface, ReferenceMap);
	}
	
	// Material Function도 처리 (UMaterialFunction은 UMaterialInterface를 상속받지 않지만 비슷한 구조)
	if (UMaterialFunction* MaterialFunction = Cast<UMaterialFunction>(Asset))
	{
		// Material Function도 Material과 비슷한 구조이므로 UpdateMaterialAssetReferences로 처리
		// UMaterialFunction을 UMaterialInterface로 캐스팅하여 처리 (null이 될 수 있지만 함수 내에서 처리)
		return UpdateMaterialAssetReferences(Cast<UMaterialInterface>(MaterialFunction), ReferenceMap);
	}
	
	bool bHasChanges = false;
	
	// 1. SoftObjectPath 참조 업데이트 (Property reflection 사용)
	UClass* AssetClass = Asset->GetClass();
	for (TFieldIterator<FProperty> PropIt(AssetClass); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		
		// SoftObjectPath 타입 프로퍼티 찾기
		if (FSoftObjectProperty* SoftObjectProp = CastField<FSoftObjectProperty>(Property))
		{
			void* PropertyValue = SoftObjectProp->ContainerPtrToValuePtr<void>(Asset);
			FSoftObjectPath* SoftPathPtr = reinterpret_cast<FSoftObjectPath*>(PropertyValue);
			
			// ReferenceMap에서 일치하는 경로 찾기
			if (const FSoftObjectPath* NewPath = ReferenceMap.Find(*SoftPathPtr))
			{
				if (*SoftPathPtr != *NewPath)
				{
					*SoftPathPtr = *NewPath;
					bHasChanges = true;
					UE_LOG(LogTemp, Verbose, TEXT("Updated SoftObjectPath reference: %s -> %s in property %s"), 
						*SoftPathPtr->ToString(), *NewPath->ToString(), *Property->GetName());
				}
			}
		}
		// SoftObjectPtr 타입 프로퍼티 찾기 (UE5에서는 FSoftObjectProperty로 통합됨)
		// FSoftObjectProperty가 SoftObjectPtr도 처리하므로 별도 처리 불필요
		// TArray<FSoftObjectPath> 타입 프로퍼티 찾기
		else if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(Property))
		{
			if (FSoftObjectProperty* InnerSoftProp = CastField<FSoftObjectProperty>(ArrayProp->Inner))
			{
				FScriptArrayHelper ArrayHelper(ArrayProp, ArrayProp->ContainerPtrToValuePtr<void>(Asset));
				for (int32 i = 0; i < ArrayHelper.Num(); ++i)
				{
					FSoftObjectPath* SoftPathPtr = reinterpret_cast<FSoftObjectPath*>(ArrayHelper.GetRawPtr(i));
					if (const FSoftObjectPath* NewPath = ReferenceMap.Find(*SoftPathPtr))
					{
						if (*SoftPathPtr != *NewPath)
						{
							*SoftPathPtr = *NewPath;
							bHasChanges = true;
							UE_LOG(LogTemp, Verbose, TEXT("Updated SoftObjectPath in array[%d]: %s -> %s in property %s"), 
								i, *SoftPathPtr->ToString(), *NewPath->ToString(), *Property->GetName());
						}
					}
				}
			}
		}
	}
	
	// 2. UObject* 직접 참조 업데이트 (FArchiveReplaceObjectRef 사용)
	TMap<UObject*, UObject*> ObjectReferenceMap;
	
	for (const auto& Pair : ReferenceMap)
	{
		UObject* OldObject = Pair.Key.TryLoad();
		UObject* NewObject = Pair.Value.TryLoad();
		
		if (OldObject && NewObject && OldObject != NewObject)
		{
			ObjectReferenceMap.Add(OldObject, NewObject);
		}
	}
	
	if (ObjectReferenceMap.Num() > 0)
	{
		// FArchiveReplaceObjectRef를 사용하여 UObject* 참조 업데이트
		FArchiveReplaceObjectRef<UObject> ReplaceAr(Asset, ObjectReferenceMap, false, true, true, true);
		bHasChanges = true;
	}
	
	if (!bHasChanges)
	{
		return true; // 변경사항이 없음
	}
	
	// 에셋 저장
	Asset->MarkPackageDirty();
	
	UPackage* Package = Asset->GetOutermost();
	if (Package)
	{
		FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
		
		// FSavePackageArgs를 사용하여 패키지 저장
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		
		UPackage::SavePackage(Package, Asset, *PackageFileName, SaveArgs);
		UE_LOG(LogTemp, Log, TEXT("Updated references and saved asset: %s"), *AssetPath.ToString());
	}
	
	return true;
}

bool FXAssetMover::UpdateNiagaraAssetReferences(
	UNiagaraSystem* NiagaraSystem,
	const TMap<FSoftObjectPath, FSoftObjectPath>& ReferenceMap)
{
	if (!NiagaraSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateNiagaraAssetReferences: NiagaraSystem is null"));
		return false;
	}

	if (ReferenceMap.Num() == 0)
	{
		return true; // 업데이트할 참조가 없음
	}

	bool bHasChanges = false;

	// 1. 모든 Emitter 순회
	TArray<FNiagaraEmitterHandle>& EmitterHandles = NiagaraSystem->GetEmitterHandles();
	for (FNiagaraEmitterHandle& EmitterHandle : EmitterHandles)
	{
		if (!EmitterHandle.IsValid())
		{
			continue;
		}

		FVersionedNiagaraEmitterData* EmitterData = EmitterHandle.GetEmitterData();
		if (!EmitterData)
		{
			continue;
		}

		// 2. 각 Emitter의 Renderer Properties 순회
		// GetRenderers()는 const 메서드이므로 직접 범위 기반 for 루프 사용
		for (UNiagaraRendererProperties* RendererProps : EmitterData->GetRenderers())
		{
			if (!RendererProps)
			{
				continue;
			}

			// 3. Renderer Properties의 모든 Property를 순회하여 Material 참조 찾기
			UClass* RendererClass = RendererProps->GetClass();
			for (TFieldIterator<FProperty> PropIt(RendererClass); PropIt; ++PropIt)
			{
				FProperty* Property = *PropIt;

				// SoftObjectPath 타입 프로퍼티 찾기 (Material 참조는 보통 SoftObjectPath로 저장됨)
				if (FSoftObjectProperty* SoftObjectProp = CastField<FSoftObjectProperty>(Property))
				{
					void* PropertyValue = SoftObjectProp->ContainerPtrToValuePtr<void>(RendererProps);
					FSoftObjectPath* SoftPathPtr = reinterpret_cast<FSoftObjectPath*>(PropertyValue);

					// ReferenceMap에서 일치하는 경로 찾기
					if (const FSoftObjectPath* NewPath = ReferenceMap.Find(*SoftPathPtr))
					{
						if (*SoftPathPtr != *NewPath)
						{
							*SoftPathPtr = *NewPath;
							bHasChanges = true;
							UE_LOG(LogTemp, Verbose, TEXT("Updated Niagara Material reference in %s: %s -> %s"), 
								*RendererProps->GetClass()->GetName(), 
								*SoftPathPtr->ToString(), 
								*NewPath->ToString());
						}
					}
				}
				// ObjectProperty 타입도 확인 (직접 Material 참조)
				else if (FObjectProperty* ObjectProp = CastField<FObjectProperty>(Property))
				{
					// Material 관련 타입인지 확인
					if (ObjectProp->PropertyClass && 
						ObjectProp->PropertyClass->IsChildOf(UMaterialInterface::StaticClass()))
					{
						void* PropertyValue = ObjectProp->ContainerPtrToValuePtr<void>(RendererProps);
						UObject** ObjectPtr = reinterpret_cast<UObject**>(PropertyValue);
						
						if (*ObjectPtr)
						{
							FSoftObjectPath CurrentPath(*ObjectPtr);
							if (const FSoftObjectPath* NewPath = ReferenceMap.Find(CurrentPath))
							{
								UObject* NewMaterial = NewPath->TryLoad();
								if (NewMaterial && *ObjectPtr != NewMaterial)
								{
									*ObjectPtr = NewMaterial;
									bHasChanges = true;
									UE_LOG(LogTemp, Verbose, TEXT("Updated Niagara Material object reference in %s: %s -> %s"), 
										*RendererProps->GetClass()->GetName(), 
										*CurrentPath.ToString(), 
										*NewPath->ToString());
								}
							}
						}
					}
				}
				// TArray<FSoftObjectPath> 타입 프로퍼티 찾기
				else if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(Property))
				{
					if (FSoftObjectProperty* InnerSoftProp = CastField<FSoftObjectProperty>(ArrayProp->Inner))
					{
						FScriptArrayHelper ArrayHelper(ArrayProp, ArrayProp->ContainerPtrToValuePtr<void>(RendererProps));
						for (int32 i = 0; i < ArrayHelper.Num(); ++i)
						{
							FSoftObjectPath* SoftPathPtr = reinterpret_cast<FSoftObjectPath*>(ArrayHelper.GetRawPtr(i));
							if (const FSoftObjectPath* NewPath = ReferenceMap.Find(*SoftPathPtr))
							{
								if (*SoftPathPtr != *NewPath)
								{
									*SoftPathPtr = *NewPath;
									bHasChanges = true;
									UE_LOG(LogTemp, Verbose, TEXT("Updated Niagara Material reference in array[%d] of %s: %s -> %s"), 
										i, *RendererProps->GetClass()->GetName(),
										*SoftPathPtr->ToString(), *NewPath->ToString());
								}
							}
						}
					}
				}
			}
		}
	}

	// 4. UObject* 직접 참조 업데이트 (FArchiveReplaceObjectRef 사용)
	TMap<UObject*, UObject*> ObjectReferenceMap;

	for (const auto& Pair : ReferenceMap)
	{
		UObject* OldObject = Pair.Key.TryLoad();
		UObject* NewObject = Pair.Value.TryLoad();

		if (OldObject && NewObject && OldObject != NewObject)
		{
			ObjectReferenceMap.Add(OldObject, NewObject);
		}
	}

	if (ObjectReferenceMap.Num() > 0)
	{
		// FArchiveReplaceObjectRef를 사용하여 UObject* 참조 업데이트
		FArchiveReplaceObjectRef<UObject> ReplaceAr(NiagaraSystem, ObjectReferenceMap, false, true, true, true);
		bHasChanges = true;
	}

	if (!bHasChanges)
	{
		return true; // 변경사항이 없음
	}

	// 에셋 저장
	NiagaraSystem->MarkPackageDirty();

	UPackage* Package = NiagaraSystem->GetOutermost();
	if (Package)
	{
		FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());

		// FSavePackageArgs를 사용하여 패키지 저장
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;

		UPackage::SavePackage(Package, NiagaraSystem, *PackageFileName, SaveArgs);
		UE_LOG(LogTemp, Log, TEXT("Updated Niagara asset references and saved: %s"), *NiagaraSystem->GetPathName());
	}

	return true;
}

bool FXAssetMover::UpdateMaterialAssetReferences(
	UMaterialInterface* MaterialInterface,
	const TMap<FSoftObjectPath, FSoftObjectPath>& ReferenceMap)
{
	if (!MaterialInterface)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateMaterialAssetReferences: MaterialInterface is null"));
		return false;
	}

	if (ReferenceMap.Num() == 0)
	{
		return true; // 업데이트할 참조가 없음
	}

	bool bHasChanges = false;

	// UMaterial 또는 UMaterialInstanceConstant으로 캐스팅
	UMaterial* Material = Cast<UMaterial>(MaterialInterface);
	UMaterialInstanceConstant* MaterialInstance = Cast<UMaterialInstanceConstant>(MaterialInterface);
	UMaterialFunction* MaterialFunction = Cast<UMaterialFunction>(MaterialInterface);

	// Material Expression들을 순회할 Material 가져오기
	UMaterial* BaseMaterial = Material;
	if (MaterialInstance)
	{
		BaseMaterial = MaterialInstance->GetBaseMaterial();
	}
	else if (MaterialFunction)
	{
		// Material Function도 Expressions를 가지고 있으므로 처리
		// Material Function은 Material과 비슷한 구조이지만 UMaterial이 아님
		// 하지만 Expressions 배열을 가지고 있으므로 직접 처리
		BaseMaterial = nullptr; // Material Function은 별도 처리
	}

	if (!BaseMaterial)
	{
		// Material Instance의 경우 Parent Material 참조 업데이트
		if (MaterialInstance)
		{
			UMaterialInterface* ParentMaterial = MaterialInstance->Parent;
			if (ParentMaterial)
			{
				FSoftObjectPath ParentPath(ParentMaterial);
				if (const FSoftObjectPath* NewParentPath = ReferenceMap.Find(ParentPath))
				{
					UMaterialInterface* NewParent = Cast<UMaterialInterface>(NewParentPath->TryLoad());
					if (NewParent && ParentMaterial != NewParent)
					{
						MaterialInstance->Parent = NewParent;
						bHasChanges = true;
						UE_LOG(LogTemp, Verbose, TEXT("Updated Material Instance Parent: %s -> %s"),
							*ParentPath.ToString(), *NewParentPath->ToString());
					}
				}
			}
		}

		// Material이 없으면 일반 Property reflection으로 처리
		// (아래 일반 로직으로 처리됨)
	}
	
	// Material Function의 Expressions 처리
	// Material Function은 FArchiveReplaceObjectRef로 처리 (Expressions 접근이 복잡함)
	// Material Function도 Material과 비슷하므로 나중에 FArchiveReplaceObjectRef로 처리
	if (BaseMaterial)
	{
		// 1. Material Expression들을 순회하여 참조 찾기
		// UE5에서는 GetExpressionCollection()을 통해 접근 시도
		// 실패하면 Property reflection 사용
		TArray<UMaterialExpression*> Expressions;
		
		// UE5 방식 - GetExpressionCollection() 메서드를 통해 접근
		FStructProperty* ExpressionCollectionProp = FindFProperty<FStructProperty>(BaseMaterial->GetClass(), TEXT("ExpressionCollection"));
		if (ExpressionCollectionProp)
		{
			void* CollectionPtr = ExpressionCollectionProp->ContainerPtrToValuePtr<void>(BaseMaterial);
			FArrayProperty* ExpressionsArrayProp = FindFProperty<FArrayProperty>(ExpressionCollectionProp->Struct, TEXT("Expressions"));
			if (ExpressionsArrayProp && CollectionPtr)
			{
				FScriptArrayHelper ArrayHelper(ExpressionsArrayProp, ExpressionsArrayProp->ContainerPtrToValuePtr<void>(CollectionPtr));
				for (int32 i = 0; i < ArrayHelper.Num(); ++i)
				{
					FObjectProperty* ExpressionProp = CastField<FObjectProperty>(ExpressionsArrayProp->Inner);
					if (ExpressionProp)
					{
						UObject** ExpressionPtr = reinterpret_cast<UObject**>(ArrayHelper.GetRawPtr(i));
						if (UMaterialExpression* Expression = Cast<UMaterialExpression>(*ExpressionPtr))
						{
							Expressions.Add(Expression);
						}
					}
				}
			}
		}
		
		// 위 방법이 실패한 경우 직접 Expressions 배열 접근 시도
		if (Expressions.Num() == 0)
		{
			FArrayProperty* ExpressionsProp = FindFProperty<FArrayProperty>(BaseMaterial->GetClass(), TEXT("Expressions"));
			// UE4 호환 코드 (주석 처리)
			// if (!ExpressionsProp)
			// {
			// 	ExpressionsProp = FindField<FArrayProperty>(BaseMaterial->GetClass(), TEXT("Expressions"));
			// }
			if (ExpressionsProp)
			{
				FScriptArrayHelper ArrayHelper(ExpressionsProp, ExpressionsProp->ContainerPtrToValuePtr<void>(BaseMaterial));
				for (int32 i = 0; i < ArrayHelper.Num(); ++i)
				{
					FObjectProperty* ExpressionProp = CastField<FObjectProperty>(ExpressionsProp->Inner);
					// UE4 호환 코드 (주석 처리)
					// if (!ExpressionProp)
					// {
					// 	ExpressionProp = CastField<UObjectProperty>(ExpressionsProp->Inner);
					// }
					if (ExpressionProp)
					{
						UObject** ExpressionPtr = reinterpret_cast<UObject**>(ArrayHelper.GetRawPtr(i));
						if (UMaterialExpression* Expression = Cast<UMaterialExpression>(*ExpressionPtr))
						{
							Expressions.Add(Expression);
						}
					}
				}
			}
		}
		
		for (UMaterialExpression* Expression : Expressions)
		{
			if (!Expression)
			{
				continue;
			}

			// Material Function Call Expression
			if (UMaterialExpressionMaterialFunctionCall* FunctionCallExpr = Cast<UMaterialExpressionMaterialFunctionCall>(Expression))
			{
				if (FunctionCallExpr->MaterialFunction)
				{
					FSoftObjectPath FunctionPath(FunctionCallExpr->MaterialFunction);
					if (const FSoftObjectPath* NewFunctionPath = ReferenceMap.Find(FunctionPath))
					{
						UMaterialFunction* NewFunction = Cast<UMaterialFunction>(NewFunctionPath->TryLoad());
						if (NewFunction && FunctionCallExpr->MaterialFunction != NewFunction)
						{
							FunctionCallExpr->MaterialFunction = NewFunction;
							bHasChanges = true;
							UE_LOG(LogTemp, Verbose, TEXT("Updated Material Function reference: %s -> %s"),
								*FunctionPath.ToString(), *NewFunctionPath->ToString());
						}
					}
				}
			}

			// Texture Sample Expression
			if (UMaterialExpressionTextureSample* TextureSampleExpr = Cast<UMaterialExpressionTextureSample>(Expression))
			{
				if (TextureSampleExpr->Texture)
				{
					FSoftObjectPath TexturePath(TextureSampleExpr->Texture);
					if (const FSoftObjectPath* NewTexturePath = ReferenceMap.Find(TexturePath))
					{
						UTexture* NewTexture = Cast<UTexture>(NewTexturePath->TryLoad());
						if (NewTexture && TextureSampleExpr->Texture != NewTexture)
						{
							TextureSampleExpr->Texture = NewTexture;
							bHasChanges = true;
							UE_LOG(LogTemp, Verbose, TEXT("Updated Texture reference in Material: %s -> %s"),
								*TexturePath.ToString(), *NewTexturePath->ToString());
						}
					}
				}
			}

			// Texture Sample Parameter Expression (Material Instance에서 사용)
			if (UMaterialExpressionTextureSampleParameter* TextureParamExpr = Cast<UMaterialExpressionTextureSampleParameter>(Expression))
			{
				if (TextureParamExpr->Texture)
				{
					FSoftObjectPath TexturePath(TextureParamExpr->Texture);
					if (const FSoftObjectPath* NewTexturePath = ReferenceMap.Find(TexturePath))
					{
						UTexture* NewTexture = Cast<UTexture>(NewTexturePath->TryLoad());
						if (NewTexture && TextureParamExpr->Texture != NewTexture)
						{
							TextureParamExpr->Texture = NewTexture;
							bHasChanges = true;
							UE_LOG(LogTemp, Verbose, TEXT("Updated Texture Parameter reference in Material: %s -> %s"),
								*TexturePath.ToString(), *NewTexturePath->ToString());
						}
					}
				}
			}
		}

		// 2. Material의 Function Infos 순회 (Material Function Layer에서 사용)
		// Property reflection으로 접근
		FArrayProperty* MaterialFunctionInfosProp = FindFProperty<FArrayProperty>(BaseMaterial->GetClass(), TEXT("MaterialFunctionInfos"));
		// UE4 호환 코드 (주석 처리)
		// if (!MaterialFunctionInfosProp)
		// {
		// 	MaterialFunctionInfosProp = FindField<FArrayProperty>(BaseMaterial->GetClass(), TEXT("MaterialFunctionInfos"));
		// }
		if (MaterialFunctionInfosProp)
		{
			FScriptArrayHelper ArrayHelper(MaterialFunctionInfosProp, MaterialFunctionInfosProp->ContainerPtrToValuePtr<void>(BaseMaterial));
			for (int32 i = 0; i < ArrayHelper.Num(); ++i)
			{
				void* FunctionInfoPtr = ArrayHelper.GetRawPtr(i);
				FStructProperty* FunctionInfoStructProp = CastField<FStructProperty>(MaterialFunctionInfosProp->Inner);
				if (FunctionInfoStructProp && FunctionInfoPtr)
				{
					FObjectProperty* FunctionProp = FindFProperty<FObjectProperty>(FunctionInfoStructProp->Struct, TEXT("Function"));
					// UE4 호환 코드 (주석 처리)
					// if (!FunctionProp)
					// {
					// 	FunctionProp = FindField<FObjectProperty>(FunctionInfoStructProp->Struct, TEXT("Function"));
					// }
					if (FunctionProp)
					{
						UObject** FunctionPtr = reinterpret_cast<UObject**>(FunctionProp->ContainerPtrToValuePtr<void>(FunctionInfoPtr));
						if (*FunctionPtr)
						{
							FSoftObjectPath FunctionPath(*FunctionPtr);
							if (const FSoftObjectPath* NewFunctionPath = ReferenceMap.Find(FunctionPath))
							{
								UMaterialFunctionInterface* NewFunction = Cast<UMaterialFunctionInterface>(NewFunctionPath->TryLoad());
								if (NewFunction && *FunctionPtr != NewFunction)
								{
									*FunctionPtr = NewFunction;
									bHasChanges = true;
									UE_LOG(LogTemp, Verbose, TEXT("Updated Material Function Info: %s -> %s"),
										*FunctionPath.ToString(), *NewFunctionPath->ToString());
								}
							}
						}
					}
				}
			}
		}
	}

	// 3. Material Instance의 Parameter Values 업데이트
	if (MaterialInstance)
	{
		// Texture Parameter Values - Property reflection으로 접근
		FArrayProperty* TextureParamValuesProp = FindFProperty<FArrayProperty>(MaterialInstance->GetClass(), TEXT("TextureParameterValues"));
		// UE4 호환 코드 (주석 처리)
		// if (!TextureParamValuesProp)
		// {
		// 	TextureParamValuesProp = FindField<FArrayProperty>(MaterialInstance->GetClass(), TEXT("TextureParameterValues"));
		// }
		if (TextureParamValuesProp)
		{
			FScriptArrayHelper ArrayHelper(TextureParamValuesProp, TextureParamValuesProp->ContainerPtrToValuePtr<void>(MaterialInstance));
			for (int32 i = 0; i < ArrayHelper.Num(); ++i)
			{
				void* ParamValuePtr = ArrayHelper.GetRawPtr(i);
				FStructProperty* ParamValueStructProp = CastField<FStructProperty>(TextureParamValuesProp->Inner);
				if (ParamValueStructProp && ParamValuePtr)
				{
					FObjectProperty* ValueProp = FindFProperty<FObjectProperty>(ParamValueStructProp->Struct, TEXT("ParameterValue"));
					// UE4 호환 코드 (주석 처리)
					// if (!ValueProp)
					// {
					// 	ValueProp = FindField<FObjectProperty>(ParamValueStructProp->Struct, TEXT("ParameterValue"));
					// }
					if (!ValueProp)
					{
						// 다른 이름 시도
						ValueProp = FindFProperty<FObjectProperty>(ParamValueStructProp->Struct, TEXT("Value"));
						// UE4 호환 코드 (주석 처리)
						// if (!ValueProp)
						// {
						// 	ValueProp = FindField<FObjectProperty>(ParamValueStructProp->Struct, TEXT("Value"));
						// }
					}
					if (ValueProp)
					{
						UObject** ValuePtr = reinterpret_cast<UObject**>(ValueProp->ContainerPtrToValuePtr<void>(ParamValuePtr));
						if (*ValuePtr)
						{
							FSoftObjectPath TexturePath(*ValuePtr);
							if (const FSoftObjectPath* NewTexturePath = ReferenceMap.Find(TexturePath))
							{
								UTexture* NewTexture = Cast<UTexture>(NewTexturePath->TryLoad());
								if (NewTexture && *ValuePtr != NewTexture)
								{
									*ValuePtr = NewTexture;
									bHasChanges = true;
									UE_LOG(LogTemp, Verbose, TEXT("Updated Material Instance Texture Parameter: %s -> %s"),
										*TexturePath.ToString(), *NewTexturePath->ToString());
								}
							}
						}
					}
				}
			}
		}

		// Material Function Parameter Values - Property reflection으로 접근
		FArrayProperty* FunctionParamValuesProp = FindFProperty<FArrayProperty>(MaterialInstance->GetClass(), TEXT("MaterialFunctionParameterValues"));
		// UE4 호환 코드 (주석 처리)
		// if (!FunctionParamValuesProp)
		// {
		// 	FunctionParamValuesProp = FindField<FArrayProperty>(MaterialInstance->GetClass(), TEXT("MaterialFunctionParameterValues"));
		// }
		if (FunctionParamValuesProp)
		{
			FScriptArrayHelper ArrayHelper(FunctionParamValuesProp, FunctionParamValuesProp->ContainerPtrToValuePtr<void>(MaterialInstance));
			for (int32 i = 0; i < ArrayHelper.Num(); ++i)
			{
				void* ParamValuePtr = ArrayHelper.GetRawPtr(i);
				FStructProperty* ParamValueStructProp = CastField<FStructProperty>(FunctionParamValuesProp->Inner);
				if (ParamValueStructProp && ParamValuePtr)
				{
					FObjectProperty* ValueProp = FindFProperty<FObjectProperty>(ParamValueStructProp->Struct, TEXT("ParameterValue"));
					// UE4 호환 코드 (주석 처리)
					// if (!ValueProp)
					// {
					// 	ValueProp = FindField<FObjectProperty>(ParamValueStructProp->Struct, TEXT("ParameterValue"));
					// }
					if (!ValueProp)
					{
						// 다른 이름 시도
						ValueProp = FindFProperty<FObjectProperty>(ParamValueStructProp->Struct, TEXT("Value"));
						// UE4 호환 코드 (주석 처리)
						// if (!ValueProp)
						// {
						// 	ValueProp = FindField<FObjectProperty>(ParamValueStructProp->Struct, TEXT("Value"));
						// }
					}
					if (ValueProp)
					{
						UObject** ValuePtr = reinterpret_cast<UObject**>(ValueProp->ContainerPtrToValuePtr<void>(ParamValuePtr));
						if (*ValuePtr)
						{
							FSoftObjectPath FunctionPath(*ValuePtr);
							if (const FSoftObjectPath* NewFunctionPath = ReferenceMap.Find(FunctionPath))
							{
								UMaterialFunctionInterface* NewFunction = Cast<UMaterialFunctionInterface>(NewFunctionPath->TryLoad());
								if (NewFunction && *ValuePtr != NewFunction)
								{
									*ValuePtr = NewFunction;
									bHasChanges = true;
									UE_LOG(LogTemp, Verbose, TEXT("Updated Material Instance Function Parameter: %s -> %s"),
										*FunctionPath.ToString(), *NewFunctionPath->ToString());
								}
							}
						}
					}
				}
			}
		}
	}

	// 4. Property reflection으로 일반 참조도 업데이트 (Material의 다른 Property들)
	UClass* MaterialClass = MaterialInterface->GetClass();
	for (TFieldIterator<FProperty> PropIt(MaterialClass); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;

		// SoftObjectPath 타입 프로퍼티 찾기
		if (FSoftObjectProperty* SoftObjectProp = CastField<FSoftObjectProperty>(Property))
		{
			void* PropertyValue = SoftObjectProp->ContainerPtrToValuePtr<void>(MaterialInterface);
			FSoftObjectPath* SoftPathPtr = reinterpret_cast<FSoftObjectPath*>(PropertyValue);

			if (const FSoftObjectPath* NewPath = ReferenceMap.Find(*SoftPathPtr))
			{
				if (*SoftPathPtr != *NewPath)
				{
					*SoftPathPtr = *NewPath;
					bHasChanges = true;
					UE_LOG(LogTemp, Verbose, TEXT("Updated Material SoftObjectPath reference in property %s: %s -> %s"),
						*Property->GetName(), *SoftPathPtr->ToString(), *NewPath->ToString());
				}
			}
		}
		// ObjectProperty 타입 확인
		else if (FObjectProperty* ObjectProp = CastField<FObjectProperty>(Property))
		{
			// Material Function, Texture 등 관련 타입인지 확인
			if (ObjectProp->PropertyClass && 
				(ObjectProp->PropertyClass->IsChildOf(UMaterialFunctionInterface::StaticClass()) ||
				 ObjectProp->PropertyClass->IsChildOf(UTexture::StaticClass())))
			{
				void* PropertyValue = ObjectProp->ContainerPtrToValuePtr<void>(MaterialInterface);
				UObject** ObjectPtr = reinterpret_cast<UObject**>(PropertyValue);

				if (*ObjectPtr)
				{
					FSoftObjectPath CurrentPath(*ObjectPtr);
					if (const FSoftObjectPath* NewPath = ReferenceMap.Find(CurrentPath))
					{
						UObject* NewObject = NewPath->TryLoad();
						if (NewObject && *ObjectPtr != NewObject)
						{
							*ObjectPtr = NewObject;
							bHasChanges = true;
							UE_LOG(LogTemp, Verbose, TEXT("Updated Material object reference in property %s: %s -> %s"),
								*Property->GetName(), *CurrentPath.ToString(), *NewPath->ToString());
						}
					}
				}
			}
		}
	}

	// 5. UObject* 직접 참조 업데이트 (FArchiveReplaceObjectRef 사용)
	TMap<UObject*, UObject*> ObjectReferenceMap;

	for (const auto& Pair : ReferenceMap)
	{
		UObject* OldObject = Pair.Key.TryLoad();
		UObject* NewObject = Pair.Value.TryLoad();

		if (OldObject && NewObject && OldObject != NewObject)
		{
			ObjectReferenceMap.Add(OldObject, NewObject);
		}
	}

	if (ObjectReferenceMap.Num() > 0)
	{
		// FArchiveReplaceObjectRef를 사용하여 UObject* 참조 업데이트
		FArchiveReplaceObjectRef<UObject> ReplaceAr(MaterialInterface, ObjectReferenceMap, false, true, true, true);
		bHasChanges = true;
	}

	if (!bHasChanges)
	{
		return true; // 변경사항이 없음
	}

	// 에셋 저장
	MaterialInterface->MarkPackageDirty();

	UPackage* Package = MaterialInterface->GetOutermost();
	if (Package)
	{
		FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());

		// FSavePackageArgs를 사용하여 패키지 저장
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;

		UPackage::SavePackage(Package, MaterialInterface, *PackageFileName, SaveArgs);
		UE_LOG(LogTemp, Log, TEXT("Updated Material asset references and saved: %s"), *MaterialInterface->GetPathName());
	}

	return true;
}

