#pragma once

#include "CoreMinimal.h"
#include "CommonRotator.h"
#include "FrontendUICommonRotator.generated.h"

/**
 * @class UFrontendUICommonRotator
 * @brief Project-specific rotator widget extending UCommonRotator.
 *
 * Extends Common UI's UCommonRotator with a convenience method for selecting
 * an option by display text rather than by index. This allows data objects
 * (UListDataObject_String) to sync the rotator's visual state directly from
 * their current string value without maintaining a separate index mapping.
 *
 * @section Architecture Text-Based Selection
 * UCommonRotator natively selects options by integer index.
 * SetSelectedOptionByText bridges between the string-based data model
 * (UListDataObject_String stores current value as FString/FText) and
 * the index-based selection model of the rotator widget.
 *
 * If the text is found in the rotator's label array, the corresponding
 * index is selected via SetSelectedItem. If not found (e.g., a custom
 * or out-of-range value), the text is set directly on the display text block
 * as a fallback, preserving visual accuracy without crashing.
 *
 * @section DisableNaiveTick
 * meta = (DisableNaiveTick) prevents UMG from enabling Tick unless explicitly
 * requested. All updates are event-driven via OnRotatedEvent callbacks.
 *
 * @see UListDataObject_String for the data model that drives this rotator.
 * @see UWidget_ListEntry_String for the entry widget that owns this rotator.
 */
UCLASS(Abstract,
	BlueprintType,
	meta = (DisableNaiveTick))
class FRONTENDUI_API UFrontendUICommonRotator : public UCommonRotator
{
	GENERATED_BODY()

public:
	/**
	 * @brief Selects the rotator option matching the given display text.
	 *
	 * Searches the rotator's TextLabels array for an exact text match.
	 * If found, selects the corresponding index via SetSelectedItem.
	 * If not found, sets the display text block directly as a fallback
	 * to preserve visual accuracy for custom or out-of-range values.
	 *
	 * @param InTextOption The display text of the option to select.
	 */
	void SetSelectedOptionByText(const FText& InTextOption);
};
