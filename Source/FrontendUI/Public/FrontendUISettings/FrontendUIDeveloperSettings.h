#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "FrontendUIDeveloperSettings.generated.h"

/**
 * @class UFrontendUIDeveloperSettings
 * @brief Project-wide configuration for the FrontendUI module, editable via Project Settings.
 *
 * Extends UDeveloperSettings so this class appears under
 * Project Settings → Game → Frontend UI Settings in the Unreal Editor.
 * All properties are persisted to DefaultGame.ini via the Config = Game specifier.
 *
 * @section Architecture Tag-Based Asset Registry
 * This class acts as a data registry that maps Gameplay Tags to soft asset references.
 * Using soft references (TSoftClassPtr, TSoftObjectPtr) ensures assets are not loaded
 * into memory at startup — they are streamed on demand via the AssetManager.
 *
 * This decouples the C++ systems from hard asset references, allowing designers
 * to swap widget classes or images without modifying or recompiling C++ code.
 *
 * @see UFrontendUIFunctionLibrary for the runtime lookup functions.
 * @see FrontendUIGameplayTags for the tag constants used as keys.
 */
UCLASS(Config = Game,
	defaultconfig,
	meta = (DisplayName = "Frontend UI Settings"))
class FRONTENDUI_API UFrontendUIDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * @brief Maps Gameplay Tags to soft widget class references.
	 *
	 * Key:   A tag from the FrontendUI.Widget category (e.g., FrontendUI.Widget.OptionsScreen).
	 * Value: Soft reference to the corresponding UWidget_ActivatableBase subclass.
	 *
	 * Looked up at runtime via UFrontendUIFunctionLibrary::GetWidgetClassByTag.
	 * Using TSoftClassPtr avoids loading all widget Blueprint classes at startup.
	 *
	 * ForceInlineRow: Displays the map entries inline in the Details panel
	 * rather than as collapsed rows, improving editor usability.
	 */
	UPROPERTY(Config,
		EditAnywhere,
		Category = "Widget Reference",
		meta = (ForceInlineRow, Categories = "FrontendUI.Widget"))
	TMap<FGameplayTag, TSoftClassPtr<class UWidget_ActivatableBase>> WidgetClassByTag;

	/**
	 * @brief Maps Gameplay Tags to soft texture references for the options screen.
	 *
	 * Key:   A tag from the FrontendUI.Image category (e.g., FrontendUI.Image.TestImage).
	 * Value: Soft reference to a UTexture2D displayed in the options details view.
	 *
	 * Looked up at runtime via UFrontendUIFunctionLibrary::GetOptionsSoftImageByTag.
	 * Images are loaded lazily via UCommonLazyImage when the details view displays them.
	 */
	UPROPERTY(Config,
		EditAnywhere,
		Category = "Options Image Reference",
		meta = (ForceInlineRow, Categories = "FrontendUI.Image"))
	TMap<FGameplayTag, TSoftObjectPtr<UTexture2D>> OptionsScreenSoftImageMap;
};
