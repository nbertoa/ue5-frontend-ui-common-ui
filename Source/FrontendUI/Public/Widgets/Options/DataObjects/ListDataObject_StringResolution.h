#pragma once

#include "CoreMinimal.h"
#include "Widgets/Options/DataObjects/ListDataObject_String.h"
#include "ListDataObject_StringResolution.generated.h"

/**
 * @class UListDataObject_StringResolution
 * @brief String data object specialized for screen resolution settings.
 *
 * Extends UListDataObject_String to populate its option list dynamically
 * from the platform's supported fullscreen resolutions, sorted from lowest
 * to highest. The maximum supported resolution is stored and used as both
 * the default value and as a dependency-driven forced value when fullscreen
 * mode is not active.
 *
 * @section Architecture Resolution Value Format
 * Resolutions are stored internally as "(X=1920,Y=1080)" strings, which matches
 * the format returned by UFrontendUIGameUserSettings::GetScreenResolution().
 * This ensures the dynamic getter and setter work correctly with the engine's
 * resolution property without manual format conversion at runtime.
 *
 * Display texts are formatted as "1920 x 1080" for user-friendly presentation.
 *
 * @section Dependency Integration
 * This data object is typically configured with a window mode dependency:
 * when the window mode is not fullscreen, the resolution is forced to the
 * maximum allowed value via the edit condition system in UOptionsDataRegistry.
 * MaximumAllowedResolution stores this value so the dependency condition can
 * reference it without re-querying the platform resolution list.
 *
 * @see UOptionsDataRegistry::InitVideoCollectionTab for dependency configuration.
 * @see UListDataObject_String for the base string option model.
 */
UCLASS()
class FRONTENDUI_API UListDataObject_StringResolution : public UListDataObject_String
{
	GENERATED_BODY()

public:
	/**
	 * @brief Populates the option list with all platform-supported fullscreen resolutions.
	 *
	 * Queries UKismetSystemLibrary::GetSupportedFullscreenResolutions, sorts results
	 * from lowest to highest resolution by pixel count, then adds each as a selectable
	 * option. Sets the maximum resolution as the default value for reset support.
	 *
	 * Must be called before InitDataObject (i.e., before AddChildListData adds this
	 * object to its parent collection), so options are populated when OnDataObjectInitialized
	 * reads the current value.
	 */
	void InitResolutionValues();

	/** @brief Returns the maximum supported resolution as an internal value string (e.g., "(X=1920,Y=1080)"). */
	FORCEINLINE FString GetMaximumAllowedResolution() const { return MaximumAllowedResolution; }

protected:
	//~ Begin UListDataObject_String Interface
	/**
	 * @brief After base initialization, falls back to displaying the current engine resolution
	 * if the saved resolution value is not found in the supported resolutions list.
	 * This handles cases where a previously valid resolution is no longer supported
	 * (e.g., after a monitor change).
	 */
	virtual void OnDataObjectInitialized() override;
	//~ End UListDataObject_String Interface

private:
	/**
	 * @brief Converts an FIntPoint resolution to the internal string format.
	 * Format: "(X=1920,Y=1080)" — matches UFrontendUIGameUserSettings::GetScreenResolution output.
	 *
	 * @param InResolution The resolution to convert.
	 * @return The resolution as an internal value string.
	 */
	static FString ResToValueString(const FIntPoint& InResolution);

	/**
	 * @brief Converts an FIntPoint resolution to a user-friendly display text.
	 * Format: "1920 x 1080".
	 *
	 * @param InResolution The resolution to convert.
	 * @return The resolution as a localized display FText.
	 */
	static FText ResToDisplayText(const FIntPoint& InResolution);

	/**
	 * @brief The maximum resolution string from the supported resolutions list.
	 * Used by UOptionsDataRegistry to configure the window mode dependency condition,
	 * forcing resolution to this value when not in fullscreen mode.
	 */
	FString MaximumAllowedResolution;
};
