#pragma once

#include "CoreMinimal.h"
#include "Widgets/Options/DataObjects/ListDataObject_Value.h"
#include "CommonNumericTextBlock.h"
#include "ListDataObject_Scalar.generated.h"

/**
 * @class UListDataObject_Scalar
 * @brief Data object for continuous numeric settings displayed as a slider.
 *
 * Extends UListDataObject_Value to represent float-based settings (e.g., volume,
 * gamma, sensitivity) that are controlled via UWidget_ListEntry_Scalar's slider widget.
 *
 * @section Architecture Display vs Output Range
 * The scalar data object maintains two separate value ranges:
 * - DisplayValueRange: The range shown to the user on the slider (e.g., 0–100%).
 * - OutputValueRange:  The range written to GameUserSettings (e.g., 0.0–1.0).
 *
 * GetCurrentValue maps from OutputValueRange → DisplayValueRange for UI display.
 * SetCurrentValueFromSlider maps from DisplayValueRange → OutputValueRange before writing.
 * This decoupling allows the UI to show user-friendly values (0–100)
 * while the settings layer operates in its native range (0.0–1.0).
 *
 * @section Number Formatting
 * DisplayNumericType and NumberFormattingOptions control how the current value
 * is formatted in UCommonNumericTextBlock (e.g., percentage, integer, decimal).
 * Use the static helpers NoDecimal() and WithDecimal() for common configurations.
 *
 * @see UWidget_ListEntry_Scalar for the widget that displays this data object.
 * @see UOptionsDataRegistry for how scalar data objects are created and configured.
 */
UCLASS()
class FRONTENDUI_API UListDataObject_Scalar : public UListDataObject_Value
{
	GENERATED_BODY()

public:
	// -------------------------------------------------------------------------
	// ACCESSORS (generated via LIST_DATA_ACCESSOR macro)
	// -------------------------------------------------------------------------
	LIST_DATA_ACCESSOR(TRange<float>,
	                   DisplayValueRange)
	LIST_DATA_ACCESSOR(TRange<float>,
	                   OutputValueRange)
	LIST_DATA_ACCESSOR(float,
	                   SliderStepSize)
	LIST_DATA_ACCESSOR(ECommonNumericType,
	                   DisplayNumericType)
	LIST_DATA_ACCESSOR(FCommonNumberFormattingOptions,
	                   NumberFormattingOptions)

	/**
	 * @brief Creates a number formatting options struct with no decimal places.
	 * Use for settings displayed as integers (e.g., frame rate limit, resolution scale).
	 *
	 * @return FCommonNumberFormattingOptions configured for zero decimal places.
	 */
	static FCommonNumberFormattingOptions NoDecimal();

	/**
	 * @brief Creates a number formatting options struct with a specified number of decimal places.
	 * Use for settings displayed as decimals (e.g., gamma: 2.20, sensitivity: 1.50).
	 *
	 * @param NumFracDigit Number of fractional digits to display.
	 * @return FCommonNumberFormattingOptions configured for the given decimal places.
	 */
	static FCommonNumberFormattingOptions WithDecimal(int32 NumFracDigit);

	/**
	 * @brief Returns the current value mapped to the display range for UI presentation.
	 *
	 * Reads the raw value from the getter (OutputValueRange), then maps it
	 * to DisplayValueRange so the slider and numeric text block show user-friendly values.
	 *
	 * @return Current value in DisplayValueRange, or 0.0 if no getter is assigned.
	 */
	float GetCurrentValue() const;

	/**
	 * @brief Sets the current value from the slider's display-range input.
	 *
	 * Maps InNewValue from DisplayValueRange to OutputValueRange, then
	 * writes it via the dynamic setter and broadcasts OnListDataModified.
	 * Called by UWidget_ListEntry_Scalar::OnSliderValueChanged.
	 *
	 * @param InNewValue The new slider value in DisplayValueRange.
	 */
	void SetCurrentValueFromSlider(float InNewValue);

private:
	//~ Begin UListDataObject_Base Interface
	/**
	 * @brief Returns true if the current output value differs from the default by more than 0.01.
	 * Uses a near-equality threshold to avoid floating point precision issues
	 * causing spurious "needs reset" states.
	 */
	virtual bool CanResetBackToDefaultValue() const override;

	/**
	 * @brief Resets the value to the default via the dynamic setter and broadcasts the change.
	 * @return True if the reset was successful, false if no default or setter is available.
	 */
	virtual bool TryResetBackToDefaultValue() override;

	/**
	 * @brief Re-broadcasts OnListDataModified when a dependency changes.
	 * Allows the slider widget to re-read and display the updated value
	 * after a dependency (e.g., a mode switch) affects this scalar's range or value.
	 */
	virtual void OnEditDependencyDataModified(UListDataObject_Base* ModifiedDependencyData,
	                                          EOptionsListDataModifyReason ModifyReason) override;
	//~ End UListDataObject_Base Interface

	/**
	 * @brief Converts a string representation to a float value.
	 * Used internally to compare string-based getter output with float default values.
	 *
	 * @param InString The string to convert (e.g., "0.75").
	 * @return The parsed float value, or 0.0 if parsing fails.
	 */
	static float StringToFloat(const FString& InString);

	// -------------------------------------------------------------------------
	// CONFIGURATION PROPERTIES
	// -------------------------------------------------------------------------

	/** @brief Range of values displayed to the user on the slider (e.g., 0–100). Default: 0–1. */
	TRange<float> DisplayValueRange = TRange<float>(0.0f,
	                                                1.0f);

	/** @brief Range of values written to GameUserSettings (e.g., 0.0–1.0). Default: 0–1. */
	TRange<float> OutputValueRange = TRange<float>(0.0f,
	                                               1.0f);

	/** @brief Minimum increment per slider step. Default: 0.1. */
	float SliderStepSize = 0.1f;

	/** @brief Numeric display type for UCommonNumericTextBlock (Number, Percentage, etc.). */
	ECommonNumericType DisplayNumericType = ECommonNumericType::Number;

	/** @brief Formatting options controlling decimal places and number style. */
	FCommonNumberFormattingOptions NumberFormattingOptions;
};
