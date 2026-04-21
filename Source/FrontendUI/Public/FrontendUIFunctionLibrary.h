#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "FrontendUIFunctionLibrary.generated.h"

/**
 * @class UFrontendUIFunctionLibrary
 * @brief Static utility functions for resolving tag-based asset references at runtime.
 *
 * Acts as the runtime lookup layer for UFrontendUIDeveloperSettings.
 * C++ and Blueprint systems use these functions to retrieve widget classes
 * and image references by Gameplay Tag, without directly depending on
 * UFrontendUIDeveloperSettings or knowing its internal map structure.
 *
 * @section Architecture Tag-Based Asset Resolution
 * All functions read from UFrontendUIDeveloperSettings via GetDefault<>(),
 * which returns the CDO (Class Default Object) — the authoritative instance
 * of the settings loaded from DefaultGame.ini. No world context is needed.
 *
 * @see UFrontendUIDeveloperSettings for the tag-to-asset mappings.
 * @see FrontendUIGameplayTags for the tag constants to pass as arguments.
 */
UCLASS()
class FRONTENDUI_API UFrontendUIFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Resolves a widget class from its Gameplay Tag.
	 *
	 * Looks up the tag in UFrontendUIDeveloperSettings::WidgetClassByTag and
	 * returns the corresponding soft class pointer. The caller is responsible
	 * for async-loading the class before use (e.g., via UFrontendUISubsystem::PushSoftWidgetToStackAsync).
	 *
	 * @warning Triggers a checkf crash if the tag is not found in the map.
	 *          Ensure all widget tags are registered in Project Settings → Frontend UI Settings.
	 *
	 * @param InWidgetTag A tag from the FrontendUI.Widget category.
	 * @return Soft class pointer to the corresponding UWidget_ActivatableBase subclass.
	 */
	UFUNCTION(BlueprintPure,
		Category = "FrontendUI Function Library")
	static TSoftClassPtr<class UWidget_ActivatableBase> GetWidgetClassByTag(
		UPARAM(meta = (Categories = "FrontendUI.Widget")) FGameplayTag InWidgetTag);

	/**
	 * @brief Resolves an options screen image from its Gameplay Tag.
	 *
	 * Looks up the tag in UFrontendUIDeveloperSettings::OptionsScreenSoftImageMap and
	 * returns the corresponding soft texture pointer. The texture is loaded lazily
	 * by UCommonLazyImage when the options details view displays it.
	 *
	 * @warning Triggers a checkf crash if the tag is not found in the map.
	 *          Ensure all image tags are registered in Project Settings → Frontend UI Settings.
	 *
	 * @param InImageTag A tag from the FrontendUI.Image category.
	 * @return Soft object pointer to the corresponding UTexture2D.
	 */
	UFUNCTION(BlueprintPure,
		Category = "FrontendUI Function Library")
	static TSoftObjectPtr<UTexture2D> GetOptionsSoftImageByTag(
		UPARAM(meta = (Categories = "FrontendUI.Image")) FGameplayTag InImageTag);
};
