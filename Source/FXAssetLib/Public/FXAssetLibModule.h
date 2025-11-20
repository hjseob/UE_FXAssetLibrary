// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * This is the module definition for the editor mode. You can implement custom functionality
 * as your plugin module starts up and shuts down. See IModuleInterface for more extensibility options.
 */
class FFXAssetLibModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;


	// contents browser 메뉴 등록
	void RegisterMenus();


	//FX Library 패널 탭을 생성합니다.
	TSharedRef<class SDockTab> OnSpawnFXLibraryTab(const class FSpawnTabArgs& SpawnTabArgs);

public:
	/** FX Library 탭 이름 */
	static const FName FXLibraryTabName;
};
