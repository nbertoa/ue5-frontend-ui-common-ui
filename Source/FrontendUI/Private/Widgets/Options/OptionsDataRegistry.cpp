#include "Widgets/Options/OptionsDataRegistry.h"

#include "Widgets/Options/DataObjects/ListDataObject_Collection.h"
#include "Widgets/Options/DataObjects/ListDataObject_String.h"
#include "Widgets/Options/OptionsDataInteractionHelper.h"
#include "FrontendUISettings/FrontendUIGameUserSettings.h"
#include "FrontendUIFunctionLibrary.h"
#include "FrontendUIGameplayTags.h"
#include "Widgets/Options/DataObjects/ListDataObject_Scalar.h"
#include "Widgets/Options/DataObjects/ListDataObject_StringResolution.h"
#include "Internationalization/StringTableRegistry.h"

// -------------------------------------------------------------------------
// MAKE_OPTIONS_DATA_CONTROL
// Creates a shared FOptionsDataInteractionHelper bound to a UFrontendUIGameUserSettings
// UFUNCTION. GET_FUNCTION_NAME_STRING_CHECKED validates the function exists at compile
// time, preventing silent runtime failures from typos in function name strings.
// -------------------------------------------------------------------------
#define MAKE_OPTIONS_DATA_CONTROL(SetterOrGetterFuncName) \
MakeShared<FOptionsDataInteractionHelper>(GET_FUNCTION_NAME_STRING_CHECKED(UFrontendUIGameUserSettings, SetterOrGetterFuncName))

// -------------------------------------------------------------------------
// GET_DESCRIPTION
// Retrieves a localized string from the options screen String Table.
// Using LOCTABLE centralizes all user-facing text in one asset, making
// localization and text updates trivial without recompiling C++.
// -------------------------------------------------------------------------
#define GET_DESCRIPTION(InKey) LOCTABLE("/Game/UI/StringTables/ST_OptionsScreenDescription.ST_OptionsScreenDescription", InKey)

void UOptionsDataRegistry::InitOptionsDataRegistry(ULocalPlayer* InOwningLocalPlayer)
{
	// Initialize tabs in display order: Gameplay → Audio → Video → Controls.
	// Each method creates a UListDataObject_Collection and adds it to
	// RegisteredOptionsTabCollections. The order here determines the tab order
	// in UFrontendUITabListWidgetBase.
	InitGameplayCollectionTab();
	InitAudioCollectionTab();
	InitVideoCollectionTab();
	InitControlCollectionTab();
}

TArray<UListDataObject_Base*> UOptionsDataRegistry::GetListSourceItemsBySelectedTabID(
	const FName& InSelectedTabID) const
{
	// Find the collection matching the selected tab ID using a predicate search.
	UListDataObject_Collection* const* FoundTabCollectionPtr = RegisteredOptionsTabCollections.FindByPredicate(
		[InSelectedTabID](const UListDataObject_Collection* AvailableTabCollection) -> bool
		{
			return AvailableTabCollection->GetDataID() == InSelectedTabID;
		});

	// Hard failure: an unregistered tab ID indicates a programming error —
	// all valid tab IDs come from the collections registered during initialization.
	checkf(FoundTabCollectionPtr,
	       TEXT("UOptionsDataRegistry::GetListSourceItemsBySelectedTabID — No tab found for ID: %s"),
	       *InSelectedTabID.ToString());

	UListDataObject_Collection* FoundTabCollection = *FoundTabCollectionPtr;

	TArray<UListDataObject_Base*> AllChildListItems;

	for (UListDataObject_Base* ChildListData : FoundTabCollection->GetAllChildListData())
	{
		if (!ChildListData)
		{
			continue;
		}

		// Add the child itself — includes sub-collections (category headers like "Volume", "Graphics").
		// UFrontendUICommonListView::OnIsSelectableOrNavigableInternal prevents
		// collections from being selected, so they appear as non-interactive headers.
		AllChildListItems.Add(ChildListData);

		// Recursively add all leaf data objects under sub-collections.
		if (ChildListData->HasAnyChildListData())
		{
			FindChildListDataRecursively(ChildListData,
			                             AllChildListItems);
		}
	}

	return AllChildListItems;
}

void UOptionsDataRegistry::FindChildListDataRecursively(UListDataObject_Base* InParentData,
                                                        TArray<UListDataObject_Base*>& OutFoundChildListData)
{
	if (!InParentData || !InParentData->HasAnyChildListData())
	{
		return;
	}

	for (UListDataObject_Base* SubChildListData : InParentData->GetAllChildListData())
	{
		if (!SubChildListData)
		{
			continue;
		}

		OutFoundChildListData.Add(SubChildListData);

		// Recurse into sub-collections to support arbitrarily nested hierarchies.
		if (SubChildListData->HasAnyChildListData())
		{
			FindChildListDataRecursively(SubChildListData,
			                             OutFoundChildListData);
		}
	}
}

void UOptionsDataRegistry::InitGameplayCollectionTab()
{
	UListDataObject_Collection* GameplayTabCollection = NewObject<UListDataObject_Collection>();
	GameplayTabCollection->SetDataID(FName("GameplayTabCollection"));
	GameplayTabCollection->SetDataDisplayName(FText::FromString(TEXT("Gameplay")));

	// -------------------------------------------------------------------------
	// GAME DIFFICULTY
	// String option with four preset difficulty levels.
	// SetShouldApplySettingsImmediately: difficulty takes effect without
	// waiting for the options screen to be closed.
	// -------------------------------------------------------------------------
	{
		UListDataObject_String* GameDifficulty = NewObject<UListDataObject_String>();
		GameDifficulty->SetDataID(FName("GameDifficulty"));
		GameDifficulty->SetDataDisplayName(FText::FromString(TEXT("Difficulty")));
		GameDifficulty->SetDescriptionRichText(FText::FromString(
			TEXT("Adjusts the difficulty of the game experience.\n\n")
			TEXT("<Bold>Easy:</> Focuses on the story experience. Provides the most relaxing combat.\n\n")
			TEXT("<Bold>Normal:</> Offers slightly harder combat experience\n\n") TEXT(
				"<Bold>Hard:</> Offers a much more challenging combat experience\n\n") TEXT(
				"<Bold>Very Hard:</> Provides the most challenging combat experience. Not recommended for first play through.")));
		GameDifficulty->AddDynamicOption(TEXT("Easy"),
		                                 FText::FromString(TEXT("Easy")));
		GameDifficulty->AddDynamicOption(TEXT("Normal"),
		                                 FText::FromString(TEXT("Normal")));
		GameDifficulty->AddDynamicOption(TEXT("Hard"),
		                                 FText::FromString(TEXT("Hard")));
		GameDifficulty->AddDynamicOption(TEXT("Very Hard"),
		                                 FText::FromString(TEXT("Very Hard")));
		GameDifficulty->SetDefaultValueFromString(TEXT("Normal"));
		GameDifficulty->SetDataDynamicGetter(MAKE_OPTIONS_DATA_CONTROL(GetCurrentGameDifficulty));
		GameDifficulty->SetDataDynamicSetter(MAKE_OPTIONS_DATA_CONTROL(SetCurrentGameDifficulty));
		GameDifficulty->SetShouldApplySettingsImmediately(true);

		GameplayTabCollection->AddChildListData(GameDifficulty);
	}

	// -------------------------------------------------------------------------
	// TEST IMAGE ITEM
	// Demonstrates the soft image system — the image is resolved via tag from
	// UFrontendUIDeveloperSettings and displayed in the options details view.
	// -------------------------------------------------------------------------
	{
		UListDataObject_String* TestItem = NewObject<UListDataObject_String>();
		TestItem->SetDataID(FName("TestItem"));
		TestItem->SetDataDisplayName(FText::FromString(TEXT("Test Image Item")));
		TestItem->SetSoftDescriptionImage(
			UFrontendUIFunctionLibrary::GetOptionsSoftImageByTag(FrontendUIGameplayTags::FrontendUI_Image_TestImage));
		TestItem->SetDescriptionRichText(FText::FromString(
			TEXT("The image to display can be specified in the project settings. ") TEXT(
				"It can be anything the developer assigned in there")));

		GameplayTabCollection->AddChildListData(TestItem);
	}

	RegisteredOptionsTabCollections.Add(GameplayTabCollection);
}

void UOptionsDataRegistry::InitAudioCollectionTab()
{
	UListDataObject_Collection* AudioTabCollection = NewObject<UListDataObject_Collection>();
	AudioTabCollection->SetDataID(FName("AudioTabCollection"));
	AudioTabCollection->SetDataDisplayName(FText::FromString(TEXT("Audio")));

	// -------------------------------------------------------------------------
	// VOLUME CATEGORY (sub-collection — appears as a non-selectable header)
	// -------------------------------------------------------------------------
	{
		UListDataObject_Collection* VolumeCategoryCollection = NewObject<UListDataObject_Collection>();
		VolumeCategoryCollection->SetDataID(FName("VolumeCategoryCollection"));
		VolumeCategoryCollection->SetDataDisplayName(FText::FromString(TEXT("Volume")));

		AudioTabCollection->AddChildListData(VolumeCategoryCollection);

		// Overall Volume (Scalar, 0–1 display = 0–200% output, percentage format)
		{
			UListDataObject_Scalar* OverallVolume = NewObject<UListDataObject_Scalar>();
			OverallVolume->SetDataID(FName("OverallVolume"));
			OverallVolume->SetDataDisplayName(FText::FromString(TEXT("Overall Volume")));
			OverallVolume->SetDescriptionRichText(FText::FromString(TEXT("This is description for Overall Volume")));
			OverallVolume->SetDisplayValueRange(TRange<float>(0.f,
			                                                  1.f));
			// Output range 0–2 allows boosting volume beyond 100%.
			OverallVolume->SetOutputValueRange(TRange<float>(0.f,
			                                                 2.f));
			OverallVolume->SetSliderStepSize(0.01f);
			OverallVolume->SetDefaultValueFromString(LexToString(1.f));
			OverallVolume->SetDisplayNumericType(ECommonNumericType::Percentage);
			OverallVolume->SetNumberFormattingOptions(UListDataObject_Scalar::NoDecimal());
			OverallVolume->SetDataDynamicGetter(MAKE_OPTIONS_DATA_CONTROL(GetOverallVolume));
			OverallVolume->SetDataDynamicSetter(MAKE_OPTIONS_DATA_CONTROL(SetOverallVolume));
			OverallVolume->SetShouldApplySettingsImmediately(true);

			VolumeCategoryCollection->AddChildListData(OverallVolume);
		}

		// Music Volume
		{
			UListDataObject_Scalar* MusicVolume = NewObject<UListDataObject_Scalar>();
			MusicVolume->SetDataID(FName("MusicVolume"));
			MusicVolume->SetDataDisplayName(FText::FromString(TEXT("Music Volume")));
			MusicVolume->SetDescriptionRichText(FText::FromString(TEXT("This is description for Music Volume")));
			MusicVolume->SetDisplayValueRange(TRange<float>(0.f,
			                                                1.f));
			MusicVolume->SetOutputValueRange(TRange<float>(0.f,
			                                               2.f));
			MusicVolume->SetSliderStepSize(0.01f);
			MusicVolume->SetDefaultValueFromString(LexToString(1.f));
			MusicVolume->SetDisplayNumericType(ECommonNumericType::Percentage);
			MusicVolume->SetNumberFormattingOptions(UListDataObject_Scalar::NoDecimal());
			MusicVolume->SetDataDynamicGetter(MAKE_OPTIONS_DATA_CONTROL(GetMusicVolume));
			MusicVolume->SetDataDynamicSetter(MAKE_OPTIONS_DATA_CONTROL(SetMusicVolume));
			MusicVolume->SetShouldApplySettingsImmediately(true);

			VolumeCategoryCollection->AddChildListData(MusicVolume);
		}

		// Sound FX Volume
		{
			UListDataObject_Scalar* SoundFXVolume = NewObject<UListDataObject_Scalar>();
			SoundFXVolume->SetDataID(FName("SoundFXVolume"));
			SoundFXVolume->SetDataDisplayName(FText::FromString(TEXT("Sound Effects Volume")));
			SoundFXVolume->SetDescriptionRichText(
				FText::FromString(TEXT("This is description for Sound Effects Volume")));
			SoundFXVolume->SetDisplayValueRange(TRange<float>(0.f,
			                                                  1.f));
			SoundFXVolume->SetOutputValueRange(TRange<float>(0.f,
			                                                 2.f));
			SoundFXVolume->SetSliderStepSize(0.01f);
			SoundFXVolume->SetDefaultValueFromString(LexToString(1.f));
			SoundFXVolume->SetDisplayNumericType(ECommonNumericType::Percentage);
			SoundFXVolume->SetNumberFormattingOptions(UListDataObject_Scalar::NoDecimal());
			SoundFXVolume->SetDataDynamicGetter(MAKE_OPTIONS_DATA_CONTROL(GetSoundFXVolume));
			SoundFXVolume->SetDataDynamicSetter(MAKE_OPTIONS_DATA_CONTROL(SetSoundFXVolume));
			SoundFXVolume->SetShouldApplySettingsImmediately(true);

			VolumeCategoryCollection->AddChildListData(SoundFXVolume);
		}
	}

	// -------------------------------------------------------------------------
	// SOUND CATEGORY (sub-collection)
	// -------------------------------------------------------------------------
	{
		UListDataObject_Collection* SoundCategoryCollection = NewObject<UListDataObject_Collection>();
		SoundCategoryCollection->SetDataID(FName("SoundCategoryCollection"));
		SoundCategoryCollection->SetDataDisplayName(FText::FromString(TEXT("Sound")));

		AudioTabCollection->AddChildListData(SoundCategoryCollection);

		// Allow Background Audio (Bool toggle, "Enabled"/"Disabled" display)
		{
			UListDataObject_StringBool* AllowBackgroundAudio = NewObject<UListDataObject_StringBool>();
			AllowBackgroundAudio->SetDataID(FName("AllowBackgroundAudio"));
			AllowBackgroundAudio->SetDataDisplayName(FText::FromString(TEXT("Allow Background Audio")));
			// Override default "ON"/"OFF" with more descriptive "Enabled"/"Disabled".
			AllowBackgroundAudio->OverrideTrueDisplayText(FText::FromString(TEXT("Enabled")));
			AllowBackgroundAudio->OverrideFalseDisplayText(FText::FromString(TEXT("Disabled")));
			AllowBackgroundAudio->SetFalseAsDefaultValue();
			AllowBackgroundAudio->SetDataDynamicGetter(MAKE_OPTIONS_DATA_CONTROL(GetAllowBackgroundAudio));
			AllowBackgroundAudio->SetDataDynamicSetter(MAKE_OPTIONS_DATA_CONTROL(SetAllowBackgroundAudio));
			AllowBackgroundAudio->SetShouldApplySettingsImmediately(true);

			SoundCategoryCollection->AddChildListData(AllowBackgroundAudio);
		}

		// Use HDR Audio Mode (Bool toggle)
		{
			UListDataObject_StringBool* UseHDRAudioMode = NewObject<UListDataObject_StringBool>();
			UseHDRAudioMode->SetDataID(FName("UseHDRAudioMode"));
			UseHDRAudioMode->SetDataDisplayName(FText::FromString(TEXT("Use HDR Audio Mode")));
			UseHDRAudioMode->OverrideTrueDisplayText(FText::FromString(TEXT("Enabled")));
			UseHDRAudioMode->OverrideFalseDisplayText(FText::FromString(TEXT("Disabled")));
			UseHDRAudioMode->SetFalseAsDefaultValue();
			UseHDRAudioMode->SetDataDynamicGetter(MAKE_OPTIONS_DATA_CONTROL(GetUseHDRAudioMode));
			UseHDRAudioMode->SetDataDynamicSetter(MAKE_OPTIONS_DATA_CONTROL(SetUseHDRAudioMode));
			UseHDRAudioMode->SetShouldApplySettingsImmediately(true);

			SoundCategoryCollection->AddChildListData(UseHDRAudioMode);
		}
	}

	RegisteredOptionsTabCollections.Add(AudioTabCollection);
}

void UOptionsDataRegistry::InitVideoCollectionTab()
{
	UListDataObject_Collection* VideoTabCollection = NewObject<UListDataObject_Collection>();
	VideoTabCollection->SetDataID(FName("VideoTabCollection"));
	VideoTabCollection->SetDataDisplayName(FText::FromString(TEXT("Video")));

	// Declared at scope level so it can be captured by lambda edit conditions
	// in sub-settings that depend on the window mode selection.
	UListDataObject_StringEnum* CreatedWindowMode = nullptr;

	// -------------------------------------------------------------------------
	// DISPLAY CATEGORY (sub-collection)
	// -------------------------------------------------------------------------
	{
		UListDataObject_Collection* DisplayCategoryCollection = NewObject<UListDataObject_Collection>();
		DisplayCategoryCollection->SetDataID(FName("DisplayCategoryCollection"));
		DisplayCategoryCollection->SetDataDisplayName(FText::FromString(TEXT("Display")));

		VideoTabCollection->AddChildListData(DisplayCategoryCollection);

		// -------------------------------------------------------------------------
		// PACKAGED BUILD ONLY CONDITION
		// Window mode and resolution are only adjustable in packaged builds —
		// the editor always runs in a specific mode and changing it causes instability.
		// This condition is reused for both Window Mode and Screen Resolution.
		// -------------------------------------------------------------------------
		FOptionsDataEditConditionDescriptor PackagedBuildOnlyCondition;
		PackagedBuildOnlyCondition.SetEditConditionFunc([]() -> bool
		{
			// GIsEditor is true in editor builds; GIsPlayInEditorWorld is true in PIE.
			// Both must be false for this condition to be met (packaged build only).
			const bool bIsInEditor = GIsEditor || GIsPlayInEditorWorld;
			return !bIsInEditor;
		});
		PackagedBuildOnlyCondition.SetDisabledRichReason(
			TEXT("\n\n<Disabled>This setting can only be adjusted in a packaged build.</>"));

		// Window Mode
		{
			UListDataObject_StringEnum* WindowMode = NewObject<UListDataObject_StringEnum>();
			WindowMode->SetDataID(FName("WindowMode"));
			WindowMode->SetDataDisplayName(FText::FromString(TEXT("Window Mode")));
			WindowMode->SetDescriptionRichText(GET_DESCRIPTION("WindowModeDescKey"));
			WindowMode->AddEnumOption(EWindowMode::Fullscreen,
			                          FText::FromString(TEXT("Fullscreen Mode")));
			WindowMode->AddEnumOption(EWindowMode::WindowedFullscreen,
			                          FText::FromString(TEXT("Borderless Window")));
			WindowMode->AddEnumOption(EWindowMode::Windowed,
			                          FText::FromString(TEXT("Windowed")));
			// Default to Borderless Window — the most compatible option for most users.
			WindowMode->SetDefaultValueFromEnumOption(EWindowMode::WindowedFullscreen);
			WindowMode->SetDataDynamicGetter(MAKE_OPTIONS_DATA_CONTROL(GetFullscreenMode));
			WindowMode->SetDataDynamicSetter(MAKE_OPTIONS_DATA_CONTROL(SetFullscreenMode));
			WindowMode->SetShouldApplySettingsImmediately(true);
			WindowMode->AddEditCondition(PackagedBuildOnlyCondition);

			// Cache pointer for use in resolution and V-Sync dependency lambdas below.
			CreatedWindowMode = WindowMode;

			DisplayCategoryCollection->AddChildListData(WindowMode);
		}

		// Screen Resolution
		{
			UListDataObject_StringResolution* ScreenResolution = NewObject<UListDataObject_StringResolution>();
			ScreenResolution->SetDataID(FName("ScreenResolution"));
			ScreenResolution->SetDataDisplayName(FText::FromString(TEXT("Screen Resolution")));
			ScreenResolution->SetDescriptionRichText(GET_DESCRIPTION("ScreenResolutionsDescKey"));

			// Populate resolution options before AddChildListData calls InitDataObject.
			ScreenResolution->InitResolutionValues();
			ScreenResolution->SetDataDynamicGetter(MAKE_OPTIONS_DATA_CONTROL(GetScreenResolution));
			ScreenResolution->SetDataDynamicSetter(MAKE_OPTIONS_DATA_CONTROL(SetScreenResolution));
			ScreenResolution->SetShouldApplySettingsImmediately(true);
			ScreenResolution->AddEditCondition(PackagedBuildOnlyCondition);

			// Resolution is also locked when window mode is Borderless Window —
			// it must match the desktop resolution exactly in that mode.
			FOptionsDataEditConditionDescriptor WindowModeEditCondition;
			WindowModeEditCondition.SetEditConditionFunc([CreatedWindowMode]() -> bool
			{
				const bool bIsBorderlessWindow = CreatedWindowMode->GetCurrentValueAsEnum<EWindowMode::Type>() ==
						EWindowMode::WindowedFullscreen;
				return !bIsBorderlessWindow;
			});
			WindowModeEditCondition.SetDisabledRichReason(
				TEXT(
					"\n\n<Disabled>Screen Resolution is not adjustable when the 'Window Mode' is set to Borderless Window.")
				TEXT("The value must match with the maximum allowed resolution.</>"));
			// Force to the maximum allowed resolution when Borderless Window is active.
			WindowModeEditCondition.SetDisabledForcedStringValue(ScreenResolution->GetMaximumAllowedResolution());

			ScreenResolution->AddEditCondition(WindowModeEditCondition);
			// Register window mode as a dependency — resolution re-evaluates when mode changes.
			ScreenResolution->AddEditDependencyData(CreatedWindowMode);

			DisplayCategoryCollection->AddChildListData(ScreenResolution);
		}
	}

	// -------------------------------------------------------------------------
	// GRAPHICS CATEGORY (sub-collection)
	// Contains brightness, overall quality preset, and individual quality settings.
	// Individual quality settings have bidirectional dependency with OverallQuality:
	// - Changing OverallQuality updates all individual settings (reads new values from engine).
	// - Changing an individual setting notifies OverallQuality to re-read its level
	//   (which may become "Custom" if settings no longer match a preset).
	// -------------------------------------------------------------------------
	{
		UListDataObject_Collection* GraphicsCategoryCollection = NewObject<UListDataObject_Collection>();
		GraphicsCategoryCollection->SetDataID(FName("GraphicsCategoryCollection"));
		GraphicsCategoryCollection->SetDataDisplayName(FText::FromString(TEXT("Graphics")));

		VideoTabCollection->AddChildListData(GraphicsCategoryCollection);

		// Display Gamma (Scalar — mapped to 1.7–2.7 output range, displayed as percentage)
		// The Unreal engine default gamma is 2.2, displayed as 50% with this mapping.
		{
			UListDataObject_Scalar* DisplayGamma = NewObject<UListDataObject_Scalar>();
			DisplayGamma->SetDataID(FName("DisplayGamma"));
			DisplayGamma->SetDataDisplayName(FText::FromString(TEXT("Brightness")));
			DisplayGamma->SetDescriptionRichText(GET_DESCRIPTION("DisplayGammaDescKey"));
			DisplayGamma->SetDisplayValueRange(TRange<float>(0.f,
			                                                 1.f));
			// Output range 1.7–2.7: Unreal's default is 2.2, shown as 50% in this mapping.
			DisplayGamma->SetOutputValueRange(TRange<float>(1.7f,
			                                                2.7f));
			DisplayGamma->SetSliderStepSize(0.01f);
			DisplayGamma->SetDisplayNumericType(ECommonNumericType::Percentage);
			DisplayGamma->SetNumberFormattingOptions(UListDataObject_Scalar::NoDecimal());
			DisplayGamma->SetDataDynamicGetter(MAKE_OPTIONS_DATA_CONTROL(GetCurrentDisplayGamma));
			DisplayGamma->SetDataDynamicSetter(MAKE_OPTIONS_DATA_CONTROL(SetCurrentDisplayGamma));
			DisplayGamma->SetDefaultValueFromString(LexToString(2.2f));

			GraphicsCategoryCollection->AddChildListData(DisplayGamma);
		}

		// Overall Quality preset (StringInteger, Low–Cinematic)
		// Acts as the "master" for all individual quality settings below.
		UListDataObject_StringInteger* CreatedOverallQuality = nullptr;
		{
			UListDataObject_StringInteger* OverallQuality = NewObject<UListDataObject_StringInteger>();
			OverallQuality->SetDataID(FName("OverallQuality"));
			OverallQuality->SetDataDisplayName(FText::FromString(TEXT("Overall Quality")));
			OverallQuality->SetDescriptionRichText(GET_DESCRIPTION("OverallQualityDescKey"));
			OverallQuality->AddIntegerOption(0,
			                                 FText::FromString(TEXT("Low")));
			OverallQuality->AddIntegerOption(1,
			                                 FText::FromString(TEXT("Medium")));
			OverallQuality->AddIntegerOption(2,
			                                 FText::FromString(TEXT("High")));
			OverallQuality->AddIntegerOption(3,
			                                 FText::FromString(TEXT("Epic")));
			OverallQuality->AddIntegerOption(4,
			                                 FText::FromString(TEXT("Cinematic")));
			OverallQuality->SetDataDynamicGetter(MAKE_OPTIONS_DATA_CONTROL(GetOverallScalabilityLevel));
			OverallQuality->SetDataDynamicSetter(MAKE_OPTIONS_DATA_CONTROL(SetOverallScalabilityLevel));
			OverallQuality->SetShouldApplySettingsImmediately(true);

			GraphicsCategoryCollection->AddChildListData(OverallQuality);
			CreatedOverallQuality = OverallQuality;
		}

		// 3D Resolution Scale (Scalar, percentage)
		// Only listens to OverallQuality — does not update OverallQuality back
		// (resolution scale is not part of the scalability preset system).
		{
			UListDataObject_Scalar* ResolutionScale = NewObject<UListDataObject_Scalar>();
			ResolutionScale->SetDataID(FName("ResolutionScale"));
			ResolutionScale->SetDataDisplayName(FText::FromString(TEXT("3D Resolution")));
			ResolutionScale->SetDescriptionRichText(GET_DESCRIPTION("ResolutionScaleDescKey"));
			ResolutionScale->SetDisplayValueRange(TRange<float>(0.f,
			                                                    1.f));
			ResolutionScale->SetOutputValueRange(TRange<float>(0.f,
			                                                   1.f));
			ResolutionScale->SetSliderStepSize(0.01f);
			ResolutionScale->SetDisplayNumericType(ECommonNumericType::Percentage);
			ResolutionScale->SetNumberFormattingOptions(UListDataObject_Scalar::NoDecimal());
			ResolutionScale->SetDataDynamicGetter(MAKE_OPTIONS_DATA_CONTROL(GetResolutionScaleNormalized));
			ResolutionScale->SetDataDynamicSetter(MAKE_OPTIONS_DATA_CONTROL(SetResolutionScaleNormalized));
			ResolutionScale->SetShouldApplySettingsImmediately(true);
			ResolutionScale->AddEditDependencyData(CreatedOverallQuality);

			GraphicsCategoryCollection->AddChildListData(ResolutionScale);
		}

		// Helper lambda to build quality settings with bidirectional OverallQuality dependency.
		// Each individual quality setting notifies OverallQuality when changed (so it can
		// update to "Custom" if settings no longer match a preset), and listens to
		// OverallQuality changes (so it updates when a preset is selected).
		auto AddQualitySetting = [&](const FName& DataID,
		                             const FString& DisplayName,
		                             const FText& Description,
		                             auto GetterFunc,
		                             auto SetterFunc)
		{
			UListDataObject_StringInteger* QualitySetting = NewObject<UListDataObject_StringInteger>();
			QualitySetting->SetDataID(DataID);
			QualitySetting->SetDataDisplayName(FText::FromString(DisplayName));
			QualitySetting->SetDescriptionRichText(Description);
			QualitySetting->AddIntegerOption(0,
			                                 FText::FromString(TEXT("Low")));
			QualitySetting->AddIntegerOption(1,
			                                 FText::FromString(TEXT("Medium")));
			QualitySetting->AddIntegerOption(2,
			                                 FText::FromString(TEXT("High")));
			QualitySetting->AddIntegerOption(3,
			                                 FText::FromString(TEXT("Epic")));
			QualitySetting->AddIntegerOption(4,
			                                 FText::FromString(TEXT("Cinematic")));
			QualitySetting->SetDataDynamicGetter(GetterFunc);
			QualitySetting->SetDataDynamicSetter(SetterFunc);
			QualitySetting->SetShouldApplySettingsImmediately(true);

			// Bidirectional dependency: individual ↔ overall quality.
			QualitySetting->AddEditDependencyData(CreatedOverallQuality);
			CreatedOverallQuality->AddEditDependencyData(QualitySetting);

			GraphicsCategoryCollection->AddChildListData(QualitySetting);
		};

		AddQualitySetting(FName("GlobalIlluminationQuality"),
		                  TEXT("Global Illumination"),
		                  GET_DESCRIPTION("GlobalIlluminationQualityDescKey"),
		                  MAKE_OPTIONS_DATA_CONTROL(GetGlobalIlluminationQuality),
		                  MAKE_OPTIONS_DATA_CONTROL(SetGlobalIlluminationQuality));

		AddQualitySetting(FName("ShadowQuality"),
		                  TEXT("Shadow Quality"),
		                  GET_DESCRIPTION("ShadowQualityDescKey"),
		                  MAKE_OPTIONS_DATA_CONTROL(GetShadowQuality),
		                  MAKE_OPTIONS_DATA_CONTROL(SetShadowQuality));

		AddQualitySetting(FName("AntiAliasingQuality"),
		                  TEXT("Anti Aliasing"),
		                  GET_DESCRIPTION("AntiAliasingDescKey"),
		                  MAKE_OPTIONS_DATA_CONTROL(GetAntiAliasingQuality),
		                  MAKE_OPTIONS_DATA_CONTROL(SetAntiAliasingQuality));

		AddQualitySetting(FName("ViewDistanceQuality"),
		                  TEXT("View Distance"),
		                  GET_DESCRIPTION("ViewDistanceDescKey"),
		                  MAKE_OPTIONS_DATA_CONTROL(GetViewDistanceQuality),
		                  MAKE_OPTIONS_DATA_CONTROL(SetViewDistanceQuality));

		AddQualitySetting(FName("TextureQuality"),
		                  TEXT("Texture Quality"),
		                  GET_DESCRIPTION("TextureQualityDescKey"),
		                  MAKE_OPTIONS_DATA_CONTROL(GetTextureQuality),
		                  MAKE_OPTIONS_DATA_CONTROL(SetTextureQuality));

		AddQualitySetting(FName("VisualEffectQuality"),
		                  TEXT("Visual Effect Quality"),
		                  GET_DESCRIPTION("VisualEffectQualityDescKey"),
		                  MAKE_OPTIONS_DATA_CONTROL(GetVisualEffectQuality),
		                  MAKE_OPTIONS_DATA_CONTROL(SetVisualEffectQuality));

		AddQualitySetting(FName("ReflectionQuality"),
		                  TEXT("Reflection Quality"),
		                  GET_DESCRIPTION("ReflectionQualityDescKey"),
		                  MAKE_OPTIONS_DATA_CONTROL(GetReflectionQuality),
		                  MAKE_OPTIONS_DATA_CONTROL(SetReflectionQuality));

		AddQualitySetting(FName("PostProcessingQuality"),
		                  TEXT("Post Processing Quality"),
		                  GET_DESCRIPTION("PostProcessingQualityDescKey"),
		                  MAKE_OPTIONS_DATA_CONTROL(GetPostProcessingQuality),
		                  MAKE_OPTIONS_DATA_CONTROL(SetPostProcessingQuality));
	}

	// -------------------------------------------------------------------------
	// ADVANCED GRAPHICS CATEGORY (sub-collection)
	// -------------------------------------------------------------------------
	{
		UListDataObject_Collection* AdvancedGraphicsCategoryCollection = NewObject<UListDataObject_Collection>();
		AdvancedGraphicsCategoryCollection->SetDataID(FName("AdvancedGraphicsCategoryCollection"));
		AdvancedGraphicsCategoryCollection->SetDataDisplayName(FText::FromString(TEXT("Advanced Graphics")));

		VideoTabCollection->AddChildListData(AdvancedGraphicsCategoryCollection);

		// V-Sync (Bool toggle — only available in fullscreen mode)
		{
			UListDataObject_StringBool* VerticalSync = NewObject<UListDataObject_StringBool>();
			VerticalSync->SetDataID(FName("VerticalSync"));
			VerticalSync->SetDataDisplayName(FText::FromString(TEXT("V-Sync")));
			VerticalSync->SetDescriptionRichText(GET_DESCRIPTION("VerticalSyncDescKey"));
			VerticalSync->SetDataDynamicGetter(MAKE_OPTIONS_DATA_CONTROL(IsVSyncEnabled));
			VerticalSync->SetDataDynamicSetter(MAKE_OPTIONS_DATA_CONTROL(SetVSyncEnabled));
			VerticalSync->SetFalseAsDefaultValue();
			VerticalSync->SetShouldApplySettingsImmediately(true);

			// V-Sync only works in fullscreen mode — disable it otherwise and force to false.
			FOptionsDataEditConditionDescriptor FullscreenOnlyCondition;
			FullscreenOnlyCondition.SetEditConditionFunc([CreatedWindowMode]() -> bool
			{
				return CreatedWindowMode->GetCurrentValueAsEnum<EWindowMode::Type>() == EWindowMode::Fullscreen;
			});
			FullscreenOnlyCondition.SetDisabledRichReason(
				TEXT("\n\n<Disabled>This feature only works if the 'Window Mode' is set to 'Fullscreen'.</>"));
			FullscreenOnlyCondition.SetDisabledForcedStringValue(TEXT("false"));

			VerticalSync->AddEditCondition(FullscreenOnlyCondition);

			AdvancedGraphicsCategoryCollection->AddChildListData(VerticalSync);
		}

		// Frame Rate Limit (String presets: 30/60/90/120/No Limit)
		{
			UListDataObject_String* FrameRateLimit = NewObject<UListDataObject_String>();
			FrameRateLimit->SetDataID(FName("FrameRateLimit"));
			FrameRateLimit->SetDataDisplayName(FText::FromString(TEXT("Frame Rate Limit")));
			FrameRateLimit->SetDescriptionRichText(GET_DESCRIPTION("FrameRateLimitDescKey"));
			FrameRateLimit->AddDynamicOption(LexToString(30.f),
			                                 FText::FromString(TEXT("30 FPS")));
			FrameRateLimit->AddDynamicOption(LexToString(60.f),
			                                 FText::FromString(TEXT("60 FPS")));
			FrameRateLimit->AddDynamicOption(LexToString(90.f),
			                                 FText::FromString(TEXT("90 FPS")));
			FrameRateLimit->AddDynamicOption(LexToString(120.f),
			                                 FText::FromString(TEXT("120 FPS")));
			// 0.0 = no frame rate limit (uncapped).
			FrameRateLimit->AddDynamicOption(LexToString(0.f),
			                                 FText::FromString(TEXT("No Limit")));
			FrameRateLimit->SetDefaultValueFromString(LexToString(0.f));
			FrameRateLimit->SetDataDynamicGetter(MAKE_OPTIONS_DATA_CONTROL(GetFrameRateLimit));
			FrameRateLimit->SetDataDynamicSetter(MAKE_OPTIONS_DATA_CONTROL(SetFrameRateLimit));
			FrameRateLimit->SetShouldApplySettingsImmediately(true);

			AdvancedGraphicsCategoryCollection->AddChildListData(FrameRateLimit);
		}
	}

	RegisteredOptionsTabCollections.Add(VideoTabCollection);
}

void UOptionsDataRegistry::InitControlCollectionTab()
{
	// Currently empty — extend with key rebinding and sensitivity settings
	// in project-specific derived registries or future iterations.
	UListDataObject_Collection* ControlTabCollection = NewObject<UListDataObject_Collection>();
	ControlTabCollection->SetDataID(FName("ControlTabCollection"));
	ControlTabCollection->SetDataDisplayName(FText::FromString(TEXT("Control")));

	RegisteredOptionsTabCollections.Add(ControlTabCollection);
}

#undef MAKE_OPTIONS_DATA_CONTROL
#undef GET_DESCRIPTION
