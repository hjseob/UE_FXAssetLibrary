// Copyright Epic Games, Inc. All Rights Reserved.

#include "FXAssetLibModule.h"
#include "FXAssetLibEditorModeCommands.h"

#include "FXLibrarySettings.h"
#include "ToolMenus.h"
#include "ContentBrowserMenuContexts.h"
#include "AssetRegistry/AssetData.h"


// FX Library Panel
#include "SFXLibraryPanel.h"

// Tab Manager
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"


#define LOCTEXT_NAMESPACE "FXAssetLibModule"

const FName FFXAssetLibModule::FXLibraryTabName = TEXT("FX Library");

void FFXAssetLibModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FFXAssetLibEditorModeCommands::Register();

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FFXAssetLibModule::RegisterMenus));


    // FX Library 탭 등록
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        FXLibraryTabName,
        FOnSpawnTab::CreateRaw(this, &FFXAssetLibModule::OnSpawnFXLibraryTab))
        .SetDisplayName(LOCTEXT("FXLibraryTabTitle", "FX Library"))
        .SetTooltipText(LOCTEXT("FXLibraryTabTooltip", "Open the FX Library panel to manage Niagara effects"))
        .SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
        .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Modes"));

}

void FFXAssetLibModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FFXAssetLibEditorModeCommands::Unregister();

	if (UToolMenus::IsToolMenuUIEnabled())
	{
		UToolMenus::UnregisterOwner(this);
	}

    // FX Library 탭 등록 해제
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FXLibraryTabName);
}



// content browser 메뉴 등록
void FFXAssetLibModule::RegisterMenus()
{
    UToolMenus* TM = UToolMenus::Get();
    UToolMenu* Menu = TM->ExtendMenu("ContentBrowser.AssetContextMenu");
    FToolMenuSection& Sec = Menu->AddSection("FXLibrary", FText::FromString("FX Library"));

    Sec.AddSubMenu(
        "FXLibraryAdd",
        FText::FromString("Add to FX Library"),
        FText::FromString("Register selected Niagara assets to a category"),
        FNewToolMenuDelegate::CreateLambda([](UToolMenu* InMenu)
        {
            const auto* Ctx = InMenu->FindContext<UContentBrowserAssetContextMenuContext>();
            if (!Ctx) return;

            const UFXLibrarySettings* Settings = GetMutableDefault<UFXLibrarySettings>();
            if (!Settings) return;

            // 각 카테고리별로 메뉴 항목 생성
            for (const FFXCategoryData& CategoryData : Settings->Categories)
            {
                FName CategoryName = CategoryData.CategoryName;

                FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
                    NAME_None,
                    FText::FromName(CategoryName),
                    FText::FromName(CategoryName),
                    FSlateIcon(),
                    FUIAction(FExecuteAction::CreateLambda([CategoryName, Selected = Ctx->SelectedAssets]()
                        {
                            UFXLibrarySettings* Mutable = GetMutableDefault<UFXLibrarySettings>();
                            if (!Mutable) return;

                            // 카테고리 찾기
                            FFXCategoryData* Category = Mutable->FindCategory(CategoryName);
                            if (!Category) return;

                            // 선택된 에셋들 중 나이아가라 시스템만 추가
                            for (const FAssetData& AD : Selected)
                            {
                                if (AD.AssetClassPath.GetAssetName() == "NiagaraSystem"
                                    || AD.AssetClassPath.ToString().Contains("NiagaraSystem"))
                                {
                                    Category->Assets.AddUnique(AD.ToSoftObjectPath());
                                }
                            }

                            // 설정 저장
                            Mutable->SaveConfig();
                            Mutable->TryUpdateDefaultConfigFile();

                            UE_LOG(LogTemp, Log, TEXT("Added assets to category: %s"), *CategoryName.ToString());
                        }))
                );

                InMenu->AddMenuEntry("FXLibrary", MoveTemp(Entry));
            }
        })
    );



    // ===== 여기에 "Open FX Library" 메뉴 추가 =====
    Sec.AddMenuEntry(
        "OpenFXLibrary",
        FText::FromString("Open FX Library"),
        FText::FromString("Open the FX Library panel"),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Modes"),
        FUIAction(FExecuteAction::CreateLambda([]()
        {
            // FX Library 탭 열기
            FGlobalTabmanager::Get()->TryInvokeTab(FFXAssetLibModule::FXLibraryTabName);
        }))
    );
}

TSharedRef<class SDockTab> FFXAssetLibModule::OnSpawnFXLibraryTab(const FSpawnTabArgs& SpawnTabArgs)
{
    // FX Library 패널 생성
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SFXLibraryPanel)
        ];
}



#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFXAssetLibModule, FXAssetLib)//EditorMode)