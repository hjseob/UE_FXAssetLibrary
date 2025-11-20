// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * FX Asset Library 전처리기 정의
 * 각 파일에서 사용하는 LOCTEXT_NAMESPACE를 여기서 통합 관리
 */

// 모듈 레벨
#define FXASSETLIB_LOCTEXT_NAMESPACE "FXAssetLibModule"

// Editor Mode
#define FXASSETLIB_EDITORMODE_LOCTEXT_NAMESPACE "FXAssetLibEditorMode"
#define FXASSETLIB_EDITORMODE_COMMANDS_LOCTEXT_NAMESPACE "FXAssetLibEditorModeCommands"
#define FXASSETLIB_EDITORMODE_TOOLKIT_LOCTEXT_NAMESPACE "FXAssetLibEditorModeToolkit"

// Tools
#define FXASSETLIB_SIMPLETOOL_LOCTEXT_NAMESPACE "FXAssetLibSimpleTool"
#define FXASSETLIB_INTERACTIVETOOL_LOCTEXT_NAMESPACE "UFXAssetLibInteractiveTool"

// Panels
#define FXASSETLIB_LIBRARYPANEL_LOCTEXT_NAMESPACE "FXAssetLibLibraryPanel"
#define FXASSETLIB_REGISTPANEL_LOCTEXT_NAMESPACE "FXAssetLibRegistPanel"

