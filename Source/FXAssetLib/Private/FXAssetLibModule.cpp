// Copyright Epic Games, Inc. All Rights Reserved.

#include "FXAssetLibModule.h"
#include "FXAssetLibEditorModeCommands.h"

#include "FXLibrarySettings.h"
#include "Core/FXAssetLibConstants.h"
#include "ToolMenus.h"
#include "ContentBrowserMenuContexts.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetData.h"


// FX Library Panel
#include "View/SFXLibraryPanel.h"
#include "View/SFXAssetRegistPanel.h"

// Tab Manager
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"

#include "Framework/Application/SlateApplication.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"


#define LOCTEXT_NAMESPACE "FXAssetLibModule"

const FName FFXAssetLibModule::FXLibraryTabName = FFXAssetLibConstants::FXLibraryTabName;

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

    // Register FX Asset 메뉴 항목
    Sec.AddMenuEntry(
        "RegisterFXAsset",
        FText::FromString("Register"),
        FText::FromString("Register selected Niagara assets with custom settings"),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Plus"),
        FUIAction(FExecuteAction::CreateLambda([]()
        {
            // Content Browser 컨텍스트에서 선택된 에셋 가져오기
            FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
            TArray<FAssetData> SelectedAssets;
            ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

            if (SelectedAssets.Num() == 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("No assets selected"));
                return;
            }

            // 등록 창 생성
            TSharedRef<SWindow> RegistrationWindow = SNew(SWindow)
                .Title(FText::FromString(TEXT("Register FX Asset")))
                .SizingRule(ESizingRule::UserSized)
                .ClientSize(FVector2D(500, 400))
                .SupportsMaximize(false)
                .SupportsMinimize(false);

            RegistrationWindow->SetContent(
                SNew(SFXAssetRegistPanel)
				.SelectedAssets(SelectedAssets)
            );

            // 창을 모달로 표시
            FSlateApplication::Get().AddModalWindow(RegistrationWindow, nullptr);
        }))
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