// Fill out your copyright notice in the Description page of Project Settings.


#include "SFXLibraryPanel.h"
#include "SlateOptMacros.h"

// FX Library Settings
#include "FXLibrarySettings.h"

// Thumbnail
#include "AssetThumbnail.h"

// Niagara
#include "NiagaraSystem.h"
#include "NiagaraActor.h"
#include "NiagaraComponent.h"

// Slate Widgets
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"

// Editor
#include "Editor.h"
#include "LevelEditor.h"
#include "Selection.h"


// Content Browser
#include "ContentBrowserModule.h"


#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"



// MultiBox (툴바)
#include "Framework/MultiBox/MultiBoxBuilder.h"


// App Style for icons
#include "Styling/AppStyle.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SFXLibraryPanel::Construct(const FArguments& InArgs)
{
	LoadFromSettings();

	// UI 구성
	ChildSlot
		[
			SNew(SVerticalBox)

				// 상단: 버튼 툴바
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)
				[
					SNew(SHorizontalBox)

						// 새로고침 버튼
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0, 0, 5, 0)
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("Refresh")))
								.ToolTipText(FText::FromString(TEXT("Reload assets from settings")))
								.OnClicked(this, &SFXLibraryPanel::OnRefreshClicked)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(SHorizontalBox)

										+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Center)
										.Padding(0, 0, 5, 0)
										[
											SNew(SImage)
												.Image(FAppStyle::GetBrush("Icons.Refresh"))
										]

										+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Center)
										[
											SNew(STextBlock)
												.Text(FText::FromString(TEXT("Refresh")))
										]
								]
						]

					// Reset Niagara 버튼
					+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("Reset Selected")))
								.ToolTipText(FText::FromString(TEXT("Reset selected Niagara actors (Shift+/)")))
								.OnClicked(this, &SFXLibraryPanel::OnResetNiagaraClicked)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(SHorizontalBox)

										+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Center)
										.Padding(0, 0, 5, 0)
										[
											SNew(SImage)
												.Image(FAppStyle::GetBrush("Icons.CircleArrowLeft"))
										]

										+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Center)
										[
											SNew(STextBlock)
												.Text(FText::FromString(TEXT("Reset Selected")))
										]
								]
						]
				]

			// 메인 컨텐츠
			+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(SSplitter)
						.Orientation(Orient_Horizontal)

						// 왼쪽: 카테고리 (배경 이미지 타일)
						+ SSplitter::Slot()
						.Value(0.3f)
						[
							SNew(SBorder)
								.Padding(5)
								[
									SNew(SVerticalBox)

										+ SVerticalBox::Slot()
										.AutoHeight()
										.Padding(5)
										[
											SNew(STextBlock)
												.Text(FText::FromString(TEXT("Categories")))
												.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
										]

										+ SVerticalBox::Slot()
										.FillHeight(1.0f)
										[
											SAssignNew(CategoryList, SListView<TSharedPtr<FName>>)
												.ListItemsSource(&Categories)
												.OnGenerateRow(this, &SFXLibraryPanel::GenCatRow)
												.OnSelectionChanged(this, &SFXLibraryPanel::OnCatChanged)
												.ItemHeight(200)  // ⭐ 카테고리 타일 높이 설정
										]
								]
						]

					// 오른쪽: 에셋 타일
					+ SSplitter::Slot()
						.Value(0.7f)
						[
							SNew(SBorder)
								.Padding(5)
								[
									SNew(SVerticalBox)

										+ SVerticalBox::Slot()
										.AutoHeight()
										.Padding(5)
										[
											SNew(STextBlock)
												.Text(FText::FromString(TEXT("Niagara Assets")))
												.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
										]

										+ SVerticalBox::Slot()
										.FillHeight(1.0f)
										[
											SAssignNew(AssetTileView, STileView<TSharedPtr<FSoftObjectPath>>)
												.ListItemsSource(&Visible)
												.OnGenerateTile(this, &SFXLibraryPanel::GenAssetTile)
												.ItemWidth(100)
												.ItemHeight(100)
										]
								]
						]
				]
		];

	if (Categories.Num() > 0)
	{
		CategoryList->SetSelection(Categories[0]);
		CurrentSelectedCategory = Categories[0];
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SFXLibraryPanel::LoadFromSettings()
{
	CatsToAssets.Empty();
	Categories.Empty();
	CategoryIcons.Empty(); // 추가
	Visible.Empty();

	const UFXLibrarySettings* Settings = GetDefault<UFXLibrarySettings>();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("[FX Library] LoadFromSettings: Settings is nullptr!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[FX Library] LoadFromSettings: Loading %d categories"), Settings->Categories.Num());

	// 새로운 구조에서 데이터 로드
	for (const FFXCategoryData& CategoryData : Settings->Categories)
	{
		UE_LOG(LogTemp, Log, TEXT("[FX Library] Loading category: %s, IconPath: %s, Assets: %d"),
			*CategoryData.CategoryName.ToString(),
			*CategoryData.IconPath.ToString(),
			CategoryData.Assets.Num());

		// 카테고리 이름
		Categories.Add(MakeShared<FName>(CategoryData.CategoryName));

		// 에셋 목록
		CatsToAssets.Add(CategoryData.CategoryName, CategoryData.Assets);

		// 아이콘 경로
		CategoryIcons.Add(CategoryData.CategoryName, CategoryData.IconPath);
	}

	UE_LOG(LogTemp, Log, TEXT("[FX Library] LoadFromSettings complete. Categories: %d, Icons: %d"),
		Categories.Num(), CategoryIcons.Num());
}

void SFXLibraryPanel::SetCategory(FName Cat)
{
	Visible.Empty();

	if (const TArray<FSoftObjectPath>* Arr = CatsToAssets.Find(Cat))
	{
		for (const FSoftObjectPath& Path : *Arr)
		{
			Visible.Add(MakeShared<FSoftObjectPath>(Path));
		}
	}

	if (AssetTileView.IsValid())
	{
		AssetTileView->RequestListRefresh();
	}
}

void SFXLibraryPanel::OnCatChanged(TSharedPtr<FName> NewSel, ESelectInfo::Type SelectInfo) const
{
	if (NewSel.IsValid())
	{
		// 현재 선택 저장
		const_cast<SFXLibraryPanel*>(this)->CurrentSelectedCategory = NewSel;
		const_cast<SFXLibraryPanel*>(this)->SetCategory(*NewSel);
	}
}

FReply SFXLibraryPanel::OnRefreshClicked()
{
	// 1. Settings 다시 로드
	LoadFromSettings();

	// 2. 카테고리 리스트 갱신
	if (CategoryList.IsValid())
	{
		CategoryList->RequestListRefresh();
	}

	// 3. 이전에 선택된 카테고리 복원
	if (CurrentSelectedCategory.IsValid())
	{
		// 동일한 카테고리 찾기
		for (const TSharedPtr<FName>& Cat : Categories)
		{
			if (Cat.IsValid() && *Cat == *CurrentSelectedCategory)
			{
				CategoryList->SetSelection(Cat);
				SetCategory(*Cat);
				break;
			}
		}
	}
	else if (Categories.Num() > 0)
	{
		// 선택된 카테고리가 없으면 첫 번째 선택
		CategoryList->SetSelection(Categories[0]);
		CurrentSelectedCategory = Categories[0];
	}

	return FReply::Handled();
}

FReply SFXLibraryPanel::OnResetNiagaraClicked()
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



TSharedRef<ITableRow> SFXLibraryPanel::GenCatRow(TSharedPtr<FName> Item, const TSharedRef<STableViewBase>& Owner)
{

	if (!Item.IsValid())
	{
		return SNew(STableRow<TSharedPtr<FName>>, Owner);
	}

	FName CategoryName = *Item;

	// 배경 이미지 브러시 로드
	FSlateBrush* BackgroundBrush = nullptr;
	const FSoftObjectPath* IconPath = CategoryIcons.Find(CategoryName);

	if (IconPath && IconPath->IsValid())
	{
		UObject* LoadedObject = IconPath->TryLoad();
		if (LoadedObject)
		{
			UTexture2D* BgTexture = Cast<UTexture2D>(LoadedObject);
			if (BgTexture)
			{
				BackgroundBrush = new FSlateDynamicImageBrush(
					BgTexture,
					FVector2D(200, 120),
					BgTexture->GetFName()
				);
			}
		}
	}

	if (!BackgroundBrush)
	{
		BackgroundBrush = new FSlateColorBrush(FLinearColor(0.15f, 0.15f, 0.2f, 1.0f));
	}

	// 커스텀 TableRow 위젯 생성
	class SCategoryRowWidget : public STableRow<TSharedPtr<FName>>
	{
	public:
		SLATE_BEGIN_ARGS(SCategoryRowWidget) {}
			SLATE_ARGUMENT(TSharedPtr<FName>, CategoryName)
			SLATE_ARGUMENT(FSlateBrush*, BackgroundBrush)
			SLATE_ARGUMENT(SFXLibraryPanel*, ParentPanel)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
		{
			CategoryName = InArgs._CategoryName;
			BackgroundBrush = InArgs._BackgroundBrush;
			ParentPanel = InArgs._ParentPanel;

			STableRow<TSharedPtr<FName>>::Construct(
				STableRow<TSharedPtr<FName>>::FArguments(),
				InOwnerTable
			);
		}

		virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			STableRow<TSharedPtr<FName>>::OnMouseEnter(MyGeometry, MouseEvent);

			if (ParentPanel && CategoryName.IsValid())
			{
				
				//UE_LOG(LogTemp, Log, TEXT("HOVER!"));


				ParentPanel->HoveredCategory = CategoryName;

				// UI 갱신
				if (ParentPanel->CategoryList.IsValid())
				{
					ParentPanel->CategoryList->RequestListRefresh();
				}
			}
		}

		virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override
		{
			STableRow<TSharedPtr<FName>>::OnMouseLeave(MouseEvent);

			if (ParentPanel)
			{
				ParentPanel->HoveredCategory = nullptr;

				// UI 갱신
				if (ParentPanel->CategoryList.IsValid())
				{
					ParentPanel->CategoryList->RequestListRefresh();
				}
			}
		}

		TSharedPtr<FName> CategoryName;
		FSlateBrush* BackgroundBrush;
		SFXLibraryPanel* ParentPanel;
	};

	// 🎨 호버 상태에 따라 색상 변경
	bool bIsHovered = (HoveredCategory.IsValid() && *HoveredCategory == CategoryName);
	FLinearColor BorderColor = bIsHovered ? FLinearColor::White : FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);
	FLinearColor ImageTint = bIsHovered ? FLinearColor::White : FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // 🎨 흑백 효과

	TSharedRef<SCategoryRowWidget> RowWidget = SNew(SCategoryRowWidget, Owner)
		.CategoryName(Item)
		.BackgroundBrush(BackgroundBrush)
		.ParentPanel(this);

	RowWidget->SetContent(
		SNew(SBorder)
		.BorderImage(BackgroundBrush)
		.BorderBackgroundColor(BorderColor) // 🎨 호버 시 테두리 색상 변경
		.Padding(10)
		[
			SNew(SBox)
				.HeightOverride(120)
				[
					SNew(SOverlay)

						// 배경 이미지 (호버 시 채도 복원)
						+ SOverlay::Slot()
						[
							SNew(SImage)
								.Image(BackgroundBrush)
								.ColorAndOpacity(ImageTint) // 🎨 흑백 ↔ 컬러 전환
						]

						// 어두운 오버레이 (호버 시 투명도 감소)
						+ SOverlay::Slot()
						[
							SNew(SImage)
								.Image(new FSlateColorBrush(FLinearColor(0, 0, 0, bIsHovered ? 0.2f : 0.5f))) // 🎨 호버 시 밝아짐
						]

						// 카테고리 이름
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(FText::FromName(CategoryName))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", bIsHovered ? 20 : 18)) // 🎨 호버 시 폰트 크기 증가
								.ColorAndOpacity(FLinearColor::White)
								.Justification(ETextJustify::Center)
								.ShadowOffset(FVector2D(2, 2))
								.ShadowColorAndOpacity(FLinearColor::Black)
						]
				]
		]
	);

	return RowWidget;
}

TSharedRef<ITableRow> SFXLibraryPanel::GenAssetTile(TSharedPtr<FSoftObjectPath> Item, const TSharedRef<STableViewBase>& Owner)
{
	if (!Item.IsValid())
	{
		return SNew(STableRow<TSharedPtr<FSoftObjectPath>>, Owner);
	}

	FString AssetName = Item->GetAssetName();

	// Asset Registry에서 FAssetData 가져오기
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(*Item);

	// 썸네일 생성
	TSharedPtr<FAssetThumbnail> Thumbnail = MakeShared<FAssetThumbnail>(AssetData, 100, 100, GetThumbnailPool());



	// 타일 위젯 생성
	TSharedRef<SWidget> TileWidget =
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f))
		.Padding(5)
		[
			SNew(SVerticalBox)
				// 썸네일 이미지 (실제 에셋 미리보기)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(5)
				[
					SNew(SBox)
						.WidthOverride(100)
						.HeightOverride(100)
						[
							Thumbnail->MakeThumbnailWidget()
						]
				]
				//// 아이콘
				//+ SVerticalBox::Slot()
				//.AutoHeight()
				//.HAlign(HAlign_Center)
				//.Padding(5)
				//[
				//	SNew(SImage)
				//		.Image(FAppStyle::GetBrush("ClassIcon.ParticleSystem"))
				//		.ColorAndOpacity(FLinearColor(0.2f, 0.8f, 1.0f))
				//]

				// 에셋 이름
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(2)
				[
					SNew(STextBlock)
						.Text(FText::FromString(AssetName))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
						.Justification(ETextJustify::Center)
						.AutoWrapText(true)
				]

			// 하단 버튼 (Browse + Remove)
			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2, 5, 2, 0)
				[
					SNew(SHorizontalBox)

						// Browse 버튼
						+ SHorizontalBox::Slot()
						.FillWidth(0.5f)
						.Padding(2, 0)
						[
							SNew(SButton)
								.ButtonStyle(FAppStyle::Get(), "SimpleButton")
								.ToolTipText(FText::FromString(TEXT("Find in Content Browser")))
								.OnClicked_Lambda([this, Item]()
									{
										BrowseToAsset(Item);
										return FReply::Handled();
									})
								[
									SNew(SImage)
										.Image(FAppStyle::GetBrush("SystemWideCommands.FindInContentBrowser"))
										.ColorAndOpacity(FSlateColor::UseForeground())
								]
						]

					// Remove 버튼
					+ SHorizontalBox::Slot()
						.FillWidth(0.5f)
						.Padding(2, 0)
						[
							SNew(SButton)
								.ButtonStyle(FAppStyle::Get(), "SimpleButton")
								.ToolTipText(FText::FromString(TEXT("Remove from Library")))
								.OnClicked_Lambda([this, Item]()
									{
										RemoveAssetFromLibrary(Item);
										return FReply::Handled();
									})
								[
									SNew(SImage)
										.Image(FAppStyle::GetBrush("Icons.Delete"))
										.ColorAndOpacity(FLinearColor(1.0f, 0.3f, 0.3f))
								]
						]
				]
		];

	// 좌클릭으로 스폰
	return SNew(STableRow<TSharedPtr<FSoftObjectPath>>, Owner)
		.Padding(5)
		[
			SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "NoBorder")
				.OnClicked_Lambda([this, Item]()
					{
						SpawnNiagaraActor(Item);
						return FReply::Handled();
					})
				[
					TileWidget
				]
		];
}



// 썸네일 풀 가져오기 (Construct에서 초기화 필요)
TSharedPtr<FAssetThumbnailPool> SFXLibraryPanel::GetThumbnailPool()
{
	if (!ThumbnailPool.IsValid())
	{
		ThumbnailPool = MakeShared<FAssetThumbnailPool>(128);
	}
	return ThumbnailPool;
}



void SFXLibraryPanel::SpawnNiagaraActor(TSharedPtr<FSoftObjectPath> AssetPath)
{
	if (!AssetPath.IsValid())
	{
		return;
	}

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

	// 3. 스폰 위치 결정 (뷰포트 중앙 또는 원점)
	FVector SpawnLocation = FVector::ZeroVector;

	// 옵션: 뷰포트 카메라 앞에 스폰하려면:
	if (GEditor && GEditor->GetActiveViewport())
	{
		FViewport* ActiveViewport = GEditor->GetActiveViewport();
		FEditorViewportClient* ViewportClient = static_cast<FEditorViewportClient*>(ActiveViewport->GetClient());
		if (ViewportClient)
		{
			FVector CameraLocation = ViewportClient->GetViewLocation();
			FRotator CameraRotation = ViewportClient->GetViewRotation();

			// 카메라 앞 500 유닛 위치
			SpawnLocation = CameraLocation + CameraRotation.Vector() * 500.0f;
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

		// 6. 스폰된 액터 선택 (옵션)
		GEditor->SelectNone(false, true);
		GEditor->SelectActor(NiagaraActor, true, true);
	}
}






// 콘텐츠 브라우저에서 에셋 찾기
void SFXLibraryPanel::BrowseToAsset(TSharedPtr<FSoftObjectPath> AssetPath)
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

// FX Library에서 에셋 삭제 (수정됨!)
void SFXLibraryPanel::RemoveAssetFromLibrary(TSharedPtr<FSoftObjectPath> AssetPath)
{
	if (!AssetPath.IsValid() || !CurrentSelectedCategory.IsValid())
	{
		return;
	}

	// Settings 가져오기
	UFXLibrarySettings* Settings = GetMutableDefault<UFXLibrarySettings>();
	if (!Settings)
	{
		return;
	}

	FName CurrentCat = *CurrentSelectedCategory;

	// FFXCategoryData에서 에셋 제거 (새로운 구조)
	FFXCategoryData* Category = Settings->FindCategory(CurrentCat);
	if (Category)
	{
		int32 RemovedCount = Category->Assets.Remove(*AssetPath);

		if (RemovedCount > 0)
		{
			// 설정 저장
			Settings->SaveConfig();
			Settings->TryUpdateDefaultConfigFile();

			// UI 갱신
			LoadFromSettings();

			// 카테고리 리스트 갱신
			if (CategoryList.IsValid())
			{
				CategoryList->RequestListRefresh();
			}

			// 현재 카테고리 다시 선택
			for (const TSharedPtr<FName>& Cat : Categories)
			{
				if (Cat.IsValid() && *Cat == CurrentCat)
				{
					CategoryList->SetSelection(Cat);
					SetCategory(*Cat);
					break;
				}
			}

			UE_LOG(LogTemp, Log, TEXT("Removed asset from library: %s"), *AssetPath->ToString());
		}
	}
}





//TSharedRef<ITableRow> SFXLibraryPanel::GenAssetRow(TSharedPtr<FSoftObjectPath> Item, const TSharedRef<STableViewBase>& Owner)
//{
//	if (!Item.IsValid())
//	{
//		return SNew(STableRow<TSharedPtr<FSoftObjectPath>>, Owner);
//	}
//
//	// 에셋 이름만 표시 (썸네일 없음)
//	FString AssetName = Item->GetAssetName();
//
//	return SNew(STableRow<TSharedPtr<FSoftObjectPath>>, Owner)
//		[
//			SNew(SBorder)
//				.Padding(FMargin(5, 3))
//				[
//					SNew(STextBlock)
//						.Text(FText::FromString(AssetName))
//				]
//		];
//}