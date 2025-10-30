// Copyright Epic Games, Inc. All Rights Reserved.

#include "FXAssetLibEditorMode.h"
#include "FXAssetLibEditorModeToolkit.h"
#include "EdModeInteractiveToolsContext.h"
#include "InteractiveToolManager.h"
#include "FXAssetLibEditorModeCommands.h"


//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
// AddYourTool Step 1 - include the header file for your Tools here
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
#include "Tools/FXAssetLibSimpleTool.h"
#include "Tools/FXAssetLibInteractiveTool.h"

// step 2: register a ToolBuilder in FFXAssetLibEditorMode::Enter() below


#define LOCTEXT_NAMESPACE "FXAssetLibEditorMode"

const FEditorModeID UFXAssetLibEditorMode::EM_FXAssetLibEditorModeId = TEXT("EM_FXAssetLibEditorMode");

FString UFXAssetLibEditorMode::SimpleToolName = TEXT("FXAssetLib_ActorInfoTool");
FString UFXAssetLibEditorMode::InteractiveToolName = TEXT("FXAssetLib_MeasureDistanceTool");


UFXAssetLibEditorMode::UFXAssetLibEditorMode()
{
	FModuleManager::Get().LoadModule("EditorStyle");

	// appearance and icon in the editing mode ribbon can be customized here
	Info = FEditorModeInfo(UFXAssetLibEditorMode::EM_FXAssetLibEditorModeId,
		LOCTEXT("ModeName", "FXAssetLib"),
		FSlateIcon(),
		true);
}


UFXAssetLibEditorMode::~UFXAssetLibEditorMode()
{
}


void UFXAssetLibEditorMode::ActorSelectionChangeNotify()
{
}

void UFXAssetLibEditorMode::Enter()
{
	UEdMode::Enter();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// AddYourTool Step 2 - register the ToolBuilders for your Tools here.
	// The string name you pass to the ToolManager is used to select/activate your ToolBuilder later.
	//////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////// 
	const FFXAssetLibEditorModeCommands& SampleToolCommands = FFXAssetLibEditorModeCommands::Get();

	RegisterTool(SampleToolCommands.SimpleTool, SimpleToolName, NewObject<UFXAssetLibSimpleToolBuilder>(this));
	RegisterTool(SampleToolCommands.InteractiveTool, InteractiveToolName, NewObject<UFXAssetLibInteractiveToolBuilder>(this));

	// active tool type is not relevant here, we just set to default
	GetToolManager()->SelectActiveToolType(EToolSide::Left, SimpleToolName);
}

void UFXAssetLibEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FFXAssetLibEditorModeToolkit);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UFXAssetLibEditorMode::GetModeCommands() const
{
	return FFXAssetLibEditorModeCommands::Get().GetCommands();
}

#undef LOCTEXT_NAMESPACE
