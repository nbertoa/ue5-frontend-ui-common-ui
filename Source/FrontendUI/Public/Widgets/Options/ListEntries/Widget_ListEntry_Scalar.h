#pragma once

#include "CoreMinimal.h"
#include "Widgets/Options/ListEntries/Widget_ListEntry_Base.h"
#include "Widget_ListEntry_Scalar.generated.h"

/**
 * @class UWidget_ListEntry_Scalar
 * @brief List entry widget for continuous numeric settings controlled by a slider.
 *
 * Displays a UListDataObject_Scalar using an UAnalogSlider for value input
 * and a UCommonNumericTextBlock for formatted value display.
 *
 * @section Architecture Slider-Data Object Binding
 * The slider operates in DisplayValueRange (user-facing values, e.g., 0–100%).
 * When the slider value changes, OnSliderValueChanged calls
 * UListDataObject_Scalar::SetCurrentValueFromSlider, which maps the display
 * value back to OutputValueRange before writing to GameUserSettings.
 *
 * When the data object value changes externally (e.g., reset, dependency),
 * OnOwningListDataObjectModified reads the mapped display value via
 * UListDataObject_Scalar::GetCurrentValue and updates both the slider and
 * the numeric text block to stay in sync.
 *
 * @section Mouse Capture
 * OnSliderMouseCaptureBegin calls SelectThisEntryWidget when the user starts
 * dragging the slider. This ensures the options details view updates to show
 * this entry's description while the user is actively interacting with it.
 *
 * @see UListDataObject_Scalar for the data model this widget displays.
 * @see UWidget_ListEntry_Base for the base entry widget lifecycle.
 */
UCLASS(Abstract,
	BlueprintType,
	meta = (DisableNaiveTick))
class FRONTENDUI_API UWidget_ListEntry_Scalar : public UWidget_ListEntry_Base
{
	GENERATED_BODY()

protected:
	//~ Begin UUserWidget Interface
	/**
	 * @brief Binds slider delegates after the widget is fully initialized.
	 * OnValueChanged and OnMouseCaptureBegin are bound here rather than in
	 * OnOwningListDataObjectSet to ensure the Slate widgets are ready.
	 */
	virtual void NativeOnInitialized() override;
	//~ End UUserWidget Interface

	//~ Begin UWidget_ListEntry_Base Interface
	/**
	 * @brief Initializes the slider and numeric text block from the scalar data object.
	 *
	 * Configures slider range, step size, and initial value from the data object's
	 * display range and current value. Also sets the numeric type and formatting
	 * options on the text block for correct value display.
	 *
	 * @param InOwningListDataObject The data object assigned to this entry (cast to UListDataObject_Scalar).
	 */
	virtual void OnOwningListDataObjectSet(UListDataObject_Base* InOwningListDataObject) override;

	/**
	 * @brief Syncs the slider and numeric text block when the data object value changes.
	 *
	 * Called when the data object broadcasts OnListDataModified (e.g., after a reset
	 * or dependency-driven value change). Reads the current display value and
	 * updates both the slider position and the numeric text block.
	 *
	 * @param OwningModifiedData The data object that changed.
	 * @param ModifyReason       Why the change occurred.
	 */
	virtual void OnOwningListDataObjectModified(UListDataObject_Base* OwningModifiedData,
	                                            EOptionsListDataModifyReason ModifyReason) override;
	//~ End UWidget_ListEntry_Base Interface

private:
	/**
	 * @brief Called when the slider value changes (user dragging or programmatic update).
	 * Forwards the new display-range value to UListDataObject_Scalar::SetCurrentValueFromSlider.
	 *
	 * @param Value The new slider value in DisplayValueRange.
	 */
	UFUNCTION()
	void OnSliderValueChanged(float Value);

	/**
	 * @brief Called when the user begins dragging the slider.
	 * Selects this entry widget so the details view updates to show this setting's description.
	 */
	UFUNCTION()
	void OnSliderMouseCaptureBegin();

	/** @brief Bound numeric text block displaying the formatted current value. Must exist in Blueprint. */
	UPROPERTY(BlueprintReadOnly,
		meta = (BindWidget, AllowPrivateAccess = "true"))
	class UCommonNumericTextBlock* CommonNumeric_SettingValue;

	/** @brief Bound analog slider for user input. Must exist in Blueprint. */
	UPROPERTY(BlueprintReadOnly,
		meta = (BindWidget, AllowPrivateAccess = "true"))
	class UAnalogSlider* AnalogSlider_SettingSlider;

	/**
	 * @brief Cached reference to the scalar data object for typed access.
	 * Set in OnOwningListDataObjectSet, avoiding repeated casts on every
	 * slider value change callback.
	 */
	UPROPERTY(Transient)
	class UListDataObject_Scalar* CachedOwningScalarDataObject;
};
