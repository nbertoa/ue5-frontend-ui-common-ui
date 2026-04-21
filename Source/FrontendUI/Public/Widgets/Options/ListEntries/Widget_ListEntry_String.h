#pragma once

#include "CoreMinimal.h"
#include "Widgets/Options/ListEntries/Widget_ListEntry_Base.h"
#include "Widget_ListEntry_String.generated.h"

/**
 * @class UWidget_ListEntry_String
 * @brief List entry widget for discrete string-based settings controlled by a rotator.
 *
 * Displays a UListDataObject_String using a UFrontendUICommonRotator for option
 * cycling and two UFrontendUICommonButtonBase instances (previous/next) for
 * explicit navigation without relying on the rotator's built-in click handling.
 *
 * @section Architecture Three Input Paths for Option Selection
 * This entry supports three ways the user can change the selected option:
 *
 * 1. Previous/Next buttons (mouse/keyboard click):
 *    OnPreviousOptionButtonClicked / OnNextOptionButtonClicked →
 *    UListDataObject_String::BackToPreviousOption / AdvanceToNextOption →
 *    setter → NotifyListDataModified → OnOwningListDataObjectModified
 *
 * 2. Rotator click (mouse click on the rotator widget):
 *    CommonRotator OnClicked → SelectThisEntryWidget (selection only, no value change)
 *
 * 3. Rotator rotation (gamepad input):
 *    CommonRotator OnRotatedEvent → OnRotatorValueChanged →
 *    UListDataObject_String::OnRotatorInitiatedValueChange →
 *    setter → NotifyListDataModified → OnOwningListDataObjectModified
 *
 * Note: OnRotatorValueChanged only processes gamepad input (bUserInitiated check).
 * Mouse-initiated rotator changes are handled via the previous/next buttons instead.
 *
 * @section Editable State
 * OnToggleEditableState disables all three interactive controls (previous button,
 * rotator, next button) when the setting is locked by an edit condition.
 *
 * @see UListDataObject_String for the data model this widget displays.
 * @see UWidget_ListEntry_Base for the base entry widget lifecycle.
 */
UCLASS(Abstract,
	BlueprintType,
	meta = (DisableNaiveTick))
class FRONTENDUI_API UWidget_ListEntry_String : public UWidget_ListEntry_Base
{
	GENERATED_BODY()

protected:
	//~ Begin UUserWidget Interface
	/**
	 * @brief Binds button and rotator delegates after the widget is fully initialized.
	 * Delegates are bound here rather than in OnOwningListDataObjectSet to ensure
	 * Slate widgets are ready before binding.
	 */
	virtual void NativeOnInitialized() override;
	//~ End UUserWidget Interface

	//~ Begin UWidget_ListEntry_Base Interface
	/**
	 * @brief Initializes the rotator with the string data object's available options and current value.
	 *
	 * Populates the rotator's text label array and sets the initially selected option
	 * to match the data object's CurrentDisplayText.
	 *
	 * @param InOwningListDataObject The data object assigned to this entry (cast to UListDataObject_String).
	 */
	virtual void OnOwningListDataObjectSet(UListDataObject_Base* InOwningListDataObject) override;

	/**
	 * @brief Disables or enables all interactive controls based on the editable state.
	 *
	 * Disables the previous button, rotator, and next button when the setting
	 * is locked by an edit condition. Calls Super to also disable the display name text block.
	 *
	 * @param bIsEditable True if the setting should be interactable.
	 */
	virtual void OnToggleEditableState(bool bIsEditable) override;
	//~ End UWidget_ListEntry_Base Interface

	/**
	 * @brief Syncs the rotator's selected option when the data object value changes externally.
	 *
	 * Called when the data object broadcasts OnListDataModified (e.g., after a reset
	 * or dependency-driven forced value change). Updates the rotator's display
	 * without triggering a value change callback.
	 *
	 * @param OwningModifiedData The data object that changed.
	 * @param ModifyReason       Why the change occurred.
	 */
	virtual void OnOwningListDataObjectModified(UListDataObject_Base* OwningModifiedData,
	                                            EOptionsListDataModifyReason ModifyReason) override;

private:
	/** @brief Called when the previous option button is clicked. Wraps to last option at beginning. */
	void OnPreviousOptionButtonClicked();

	/** @brief Called when the next option button is clicked. Wraps to first option at end. */
	void OnNextOptionButtonClicked();

	/**
	 * @brief Called when the rotator's selected index changes.
	 *
	 * Only processes gamepad-initiated changes (bUserInitiated must be true and
	 * input type must be Gamepad). Mouse-initiated changes are handled via
	 * the previous/next buttons instead.
	 *
	 * @param Value         The new selected index in the rotator.
	 * @param bUserInitiated True if the change was triggered by user input.
	 */
	void OnRotatorValueChanged(int32 Value,
	                           bool bUserInitiated) const;

	/** @brief Bound previous option navigation button. Must exist in Blueprint. */
	UPROPERTY(BlueprintReadOnly,
		meta = (BindWidget, AllowPrivateAccess = "true"))
	class UFrontendUICommonButtonBase* CommonButton_PreviousOption;

	/** @brief Bound rotator widget for cycling through available options. Must exist in Blueprint. */
	UPROPERTY(BlueprintReadOnly,
		meta = (BindWidget, AllowPrivateAccess = "true"))
	class UFrontendUICommonRotator* CommonRotator_AvailableOptions;

	/** @brief Bound next option navigation button. Must exist in Blueprint. */
	UPROPERTY(BlueprintReadOnly,
		meta = (BindWidget, AllowPrivateAccess = "true"))
	class UFrontendUICommonButtonBase* CommonButton_NextOption;

	/**
	 * @brief Cached reference to the string data object for typed access.
	 * Set in OnOwningListDataObjectSet to avoid repeated casts on every
	 * button click and rotator event callback.
	 */
	UPROPERTY(Transient)
	class UListDataObject_String* CachedOwningStringDataObject;
};
