// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/SFXLibraryPanelController.h"
#include "Model/FXLibraryState.h"
#include "Model/FXLibraryModel.h"
#include "Editor.h"
#include "LevelEditor.h"
#include "Selection.h"
#include "NiagaraSystem.h"
#include "NiagaraActor.h"
#include "NiagaraComponent.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Utils/FXAssetReferenceCollector.h"
#include "Core/FXAssetLibConstants.h"

SFXLibraryPanelController::SFXLibraryPanelController()
{
}

SFXLibraryPanelController::~SFXLibraryPanelController()
{
}

void SFXLibraryPanelController::Initialize(
	TSharedPtr<FFXLibraryState> InState,
	TSharedPtr<FFXLibraryModel> InModel)
{
	State = InState;
	Model = InModel;

	if (Model.IsValid())
	{
		Model->SetState(State);
		Model->LoadFromSettings();
	}
}

FReply SFXLibraryPanelController::OnRefreshClicked()
{
	if (!Model.IsValid() || !State.IsValid())
	{
		return FReply::Handled();
	}

	// Settings에서 다시 로드
	Model->LoadFromSettings();

	return FReply::Handled();
}

FReply SFXLibraryPanelController::OnResetNiagaraClicked()
{
	if (!GEditor)
	{
		return FReply::Handled();
	}

	// 선택된 액터들 가져오기
	USelection* SelectedActors = GEditor->GetSelectedActors();
	if (!SelectedActors)
	{
		UE_LOG(LogTemp, Warning, TEXT("No actors selected"));
		return FReply::Handled();
	}

	int32 ResetCount = 0;

	// 선택된 각 액터에 대해
	for (FSelectionIterator It(*SelectedActors); It; ++It)
	{
		if (AActor* Actor = Cast<AActor>(*It))
		{
			// ANiagaraActor인 경우
			if (ANiagaraActor* NiagaraActor = Cast<ANiagaraActor>(Actor))
			{
				if (UNiagaraComponent* NiagaraComp = NiagaraActor->GetNiagaraComponent())
				{
					// 나이아가라 시스템 재실행
					NiagaraComp->Activate(true);
					ResetCount++;
				}
			}
			// 일반 액터에 UNiagaraComponent가 있는 경우
			else
			{
				TArray<UNiagaraComponent*> NiagaraComponents;
				Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);

				for (UNiagaraComponent* NiagaraComp : NiagaraComponents)
				{
					if (NiagaraComp)
					{
						NiagaraComp->Activate(true);
						ResetCount++;
					}
				}
			}
		}
	}

	if (ResetCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Reset %d Niagara component(s)"), ResetCount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Niagara actors selected"));
	}

	return FReply::Handled();
}

FReply SFXLibraryPanelController::OnCleanupClicked()
{
	if (!Model.IsValid())
	{
		return FReply::Handled();
	}

	int32 RemovedInvalidAssets = Model->CleanupInvalidAssets();
	int32 RemovedEmptyCategories = Model->CleanupEmptyCategories();

	UE_LOG(LogTemp, Warning, TEXT("Clean Up completed: Removed %d invalid assets, %d empty categories"), 
		RemovedInvalidAssets, RemovedEmptyCategories);

	return FReply::Handled();
}

void SFXLibraryPanelController::SpawnNiagaraActor(TSharedPtr<FSoftObjectPath> AssetPath)
{
	if (!AssetPath.IsValid())
	{
		return;
	}

	// 참조된 에셋 목록 수집 (필터링된 /Game/ 하위 에셋만)
	TArray<FReferencedAsset> ReferencedAssets = FFXAssetReferenceCollector::CollectAllReferences(*AssetPath);

	// 1. FSoftObjectPath에서 실제 UNiagaraSystem 로드
	UNiagaraSystem* NiagaraSystem = Cast<UNiagaraSystem>(AssetPath->TryLoad());
	if (!NiagaraSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load Niagara System: %s"), *AssetPath->ToString());
		return;
	}

	// 2. 에디터 월드 가져오기
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid editor world found"));
		return;
	}

	// 3. 스폰 위치 결정 (뷰포트 카메라 앞)
	FVector SpawnLocation = FVector::ZeroVector;

	if (GEditor && GEditor->GetActiveViewport())
	{
		FViewport* ActiveViewport = GEditor->GetActiveViewport();
		FEditorViewportClient* ViewportClient = static_cast<FEditorViewportClient*>(ActiveViewport->GetClient());
		if (ViewportClient)
		{
			FVector CameraLocation = ViewportClient->GetViewLocation();
			FRotator CameraRotation = ViewportClient->GetViewRotation();

			// 카메라 앞 거리만큼 위치
			SpawnLocation = CameraLocation + CameraRotation.Vector() * FFXAssetLibConstants::NiagaraSpawnDistance;
		}
	}

	// 4. Niagara Actor 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ANiagaraActor* NiagaraActor = World->SpawnActor<ANiagaraActor>(
		ANiagaraActor::StaticClass(),
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (NiagaraActor)
	{
		// 5. Niagara System 설정 및 활성화
		UNiagaraComponent* NiagaraComponent = NiagaraActor->GetNiagaraComponent();
		if (NiagaraComponent)
		{
			NiagaraComponent->SetAsset(NiagaraSystem);
			NiagaraComponent->Activate(true);

			UE_LOG(LogTemp, Log, TEXT("Spawned Niagara Actor: %s at %s"),
				*NiagaraSystem->GetName(),
				*SpawnLocation.ToString());
		}

		// 6. 스폰된 액터 선택
		GEditor->SelectNone(false, true);
		GEditor->SelectActor(NiagaraActor, true, true);
	}
}

void SFXLibraryPanelController::BrowseToAsset(TSharedPtr<FSoftObjectPath> AssetPath)
{
	if (!AssetPath.IsValid())
	{
		return;
	}

	// 에셋 로드
	UObject* Asset = AssetPath->TryLoad();
	if (!Asset)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load asset: %s"), *AssetPath->ToString());
		return;
	}

	// 콘텐츠 브라우저 모듈 가져오기
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	// 에셋 배열 생성
	TArray<FAssetData> AssetsToSync;
	AssetsToSync.Add(FAssetData(Asset));

	// 콘텐츠 브라우저에서 에셋으로 이동
	ContentBrowserModule.Get().SyncBrowserToAssets(AssetsToSync);

	UE_LOG(LogTemp, Log, TEXT("Browsed to asset: %s"), *AssetPath->ToString());
}

void SFXLibraryPanelController::RemoveAssetFromLibrary(TSharedPtr<FSoftObjectPath> AssetPath)
{
	if (!AssetPath.IsValid() || !State.IsValid() || !Model.IsValid())
	{
		return;
	}

	if (!State->SelectedCategory.IsValid())
	{
		return;
	}

	FName CurrentCat = *State->SelectedCategory;
	int32 RemovedCount = Model->RemoveAssetFromCategory(CurrentCat, *AssetPath);

	if (RemovedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Removed asset from library: %s"), *AssetPath->ToString());
	}
}

void SFXLibraryPanelController::OnCategoryChanged(TSharedPtr<FName> Category)
{
	if (State.IsValid())
	{
		State->SetSelectedCategory(Category);
	}
}

void SFXLibraryPanelController::OnCategoryHovered(TSharedPtr<FName> Category)
{
	if (State.IsValid())
	{
		State->SetHoveredCategory(Category);
	}
}

void SFXLibraryPanelController::OnCategoryUnhovered()
{
	if (State.IsValid())
	{
		State->SetHoveredCategory(nullptr);
	}
}

