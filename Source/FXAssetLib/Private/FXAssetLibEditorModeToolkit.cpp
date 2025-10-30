// Copyright Epic Games, Inc. All Rights Reserved.

#include "FXAssetLibEditorModeToolkit.h"
#include "FXAssetLibEditorMode.h"
#include "Engine/Selection.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "FXAssetLibEditorModeToolkit"

FFXAssetLibEditorModeToolkit::FFXAssetLibEditorModeToolkit()
{
}

void FFXAssetLibEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);
}

void FFXAssetLibEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames.Add(NAME_Default);
}


FName FFXAssetLibEditorModeToolkit::GetToolkitFName() const
{
	return FName("FXAssetLibEditorMode");
}

FText FFXAssetLibEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DisplayName", "FXAssetLibEditorMode Toolkit");
}

#undef LOCTEXT_NAMESPACE
