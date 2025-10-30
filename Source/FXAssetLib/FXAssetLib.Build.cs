// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FXAssetLib : ModuleRules
{
	public FXAssetLib(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
				"DeveloperSettings",
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"EditorFramework",
				"EditorStyle",
				"UnrealEd",
				"LevelEditor",
				"InteractiveToolsFramework",
				"EditorInteractiveToolsFramework",
				// ... add private dependencies that you statically link with here ...	

				"ToolMenus",           // <- 추가
				"ContentBrowserData",  // <- 추가
				"ContentBrowser",      // <- 추가 (필요시)
				"Niagara",                // UNiagaraSystem, ANiagaraActor 등
				"AssetTools",             // FAssetThumbnail, FAssetThumbnailPool
				"WorkspaceMenuStructure", // <- 추가! WorkspaceMenu::GetMenuStructure()
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
