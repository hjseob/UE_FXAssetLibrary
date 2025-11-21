// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/SFXAssetRegistPanelController.h"
#include "Model/FXLibraryModel.h"
#include "Model/FXLibraryState.h"
#include "Utils/FXAssetOrganizer.h"
#include "Utils/FXAssetMover.h"
#include "Core/FXAssetLibConstants.h"

SFXAssetRegistPanelController::SFXAssetRegistPanelController()
{
}

SFXAssetRegistPanelController::~SFXAssetRegistPanelController()
{
}

void SFXAssetRegistPanelController::Initialize(TSharedPtr<FFXLibraryModel> InModel)
{
	Model = InModel;
}

FReply SFXAssetRegistPanelController::OnRegisterClicked(
	const FString& RootPath,
	const FString& AssetName,
	const FString& CategoryName,
	const TArray<FAssetData>& SelectedAssets)
{
	if (!Model.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get FX Library Model"));
		return FReply::Handled();
	}

	// 유효성 검사
	if (RootPath.IsEmpty() || AssetName.IsEmpty() || CategoryName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Please fill in all required fields"));
		return FReply::Handled();
	}

	// 카테고리 찾기 또는 생성
	FFXCategoryData* Category = Model->FindCategory(*CategoryName);
	if (!Category)
	{
		Category = Model->AddCategory(*CategoryName);
		UE_LOG(LogTemp, Log, TEXT("Created new category: %s"), *CategoryName);
	}

	// 선택된 에셋들 중 나이아가라 시스템만 복사 및 등록
	int32 AddedCount = 0;
	for (const FAssetData& AssetData : SelectedAssets)
	{
		if (AssetData.AssetClassPath.GetAssetName() == "NiagaraSystem"
			|| AssetData.AssetClassPath.ToString().Contains("NiagaraSystem"))
		{
			// 원본 에셋 경로
			FSoftObjectPath SourceAssetPath = AssetData.ToSoftObjectPath();
			
			// 대상 폴더 경로 생성 (RootPath/Particles/CategoryName)
			FString DestinationFolder = FXAssetOrganizer::GetFolderPathForAssetType(
				RootPath, CategoryName, TEXT("NiagaraSystem"));
			
			// 폴더 생성
			if (!FXAssetOrganizer::CreateFolderPath(DestinationFolder))
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to create folder: %s"), *DestinationFolder);
				continue;
			}
			
			// 에셋과 모든 참조를 복사 (재귀적으로 Material, Texture 등도 복사)
			FSoftObjectPath CopiedAssetPath = FXAssetMover::CopyAssetWithReferences(
				SourceAssetPath,
				DestinationFolder,
				AssetName,
				RootPath
			);
			
			if (CopiedAssetPath.IsValid())
			{
				// 복사된 에셋을 카테고리에 추가
				if (Model->AddAssetToCategory(*CategoryName, CopiedAssetPath))
				{
					AddedCount++;
					UE_LOG(LogTemp, Log, TEXT("Copied and registered asset with references: %s -> %s"), 
						*SourceAssetPath.ToString(), *CopiedAssetPath.ToString());
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to copy asset: %s"), *SourceAssetPath.ToString());
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Registered %d assets to category: %s (Root: %s)"), 
		AddedCount, *CategoryName, *RootPath);

	return FReply::Handled();
}

FReply SFXAssetRegistPanelController::OnCancelClicked()
{
	return FReply::Handled();
}

void SFXAssetRegistPanelController::OnRootPathChanged(const FText& NewText, FString& OutRootPath)
{
	OutRootPath = NewText.ToString();
}

void SFXAssetRegistPanelController::OnAssetNameChanged(const FText& NewText, FString& OutAssetName)
{
	OutAssetName = NewText.ToString();
}

void SFXAssetRegistPanelController::OnCategorySelectionChanged(TSharedPtr<FString> NewSelection, FString& OutCategoryName)
{
	if (NewSelection.IsValid())
	{
		OutCategoryName = *NewSelection;
	}
}

void SFXAssetRegistPanelController::OnHashtagsChanged(const FText& NewText, FString& OutHashtags)
{
	OutHashtags = NewText.ToString();
}

FReply SFXAssetRegistPanelController::OnAddCategoryClicked(FString& OutNewCategoryName)
{
	// 실제 구현은 View에서 다이얼로그를 띄우고 결과를 받아옴
	// 여기서는 기본값만 설정
	OutNewCategoryName = TEXT("NewCategory");
	return FReply::Handled();
}

void SFXAssetRegistPanelController::LoadCategoryOptions(TArray<TSharedPtr<FString>>& OutCategoryOptions)
{
	OutCategoryOptions.Empty();
	
	if (!Model.IsValid())
	{
		// 기본값 추가
		OutCategoryOptions.Add(MakeShareable(new FString(FFXAssetLibConstants::DefaultCategoryName)));
		return;
	}

	// Model의 State를 통해 카테고리 목록 가져오기
	TSharedPtr<FFXLibraryState> State = Model->GetState();
	if (State.IsValid())
	{
		for (const TSharedPtr<FName>& CategoryPtr : State->Categories)
		{
			if (CategoryPtr.IsValid())
			{
				OutCategoryOptions.Add(MakeShareable(new FString(CategoryPtr->ToString())));
			}
		}
	}

	// 기본값이 없으면 추가
	if (OutCategoryOptions.Num() == 0)
	{
		OutCategoryOptions.Add(MakeShareable(new FString(FFXAssetLibConstants::DefaultCategoryName)));
	}
}

