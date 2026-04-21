#pragma once

#include "CoreMinimal.h"
#include "Widgets/Options/DataObjects/ListDataObject_Value.h"
#include "ListDataObject_String.generated.h"

/**
 * @class UListDataObject_String
 * @brief Data object for discrete string-based settings displayed as a rotator.
 *
 * Represents settings with a fixed set of options (e.g., difficulty, window mode,
 * texture quality) where the user cycles through choices using previous/next buttons
 * or a UCommonRotator widget.
 *
 * @section Architecture String-Based Option Model
 * Options are stored as parallel arrays:
 * - AvailableOptionsStringArray: internal string values written to GameUserSettings.
 * - AvailableOptionsTextArray:   localized display texts shown to the user.
 *
 * CurrentStringValue tracks the active internal value. CurrentDisplayText tracks
 * the active display text. Both are kept in sync via TrySetDisplayTextFromStringValue.
 *
 * @section Subclasses
 * - UListDataObject_StringBool:    Boolean toggle (true/false) with customizable display text.
 * - UListDataObject_StringEnum:    Template-based enum option support.
 * - UListDataObject_StringInteger: Integer options (e.g., frame rate presets).
 *
 * @see UWidget_ListEntry_String for the widget that displays this data object.
 * @see UOptionsDataRegistry for how string data objects are created and configured.
 */
UCLASS()
class FRONTENDUI_API UListDataObject_String : public UListDataObject_Value
{
	GENERATED_BODY()

public:
	/**
	 * @brief Adds a selectable option to this data object.
	 *
	 * @param InStringValue  Internal value written to GameUserSettings when selected.
	 * @param InDisplayText  Localized text displayed to the user in the rotator.
	 */
	void AddDynamicOption(const FString& InStringValue,
	                      const FText& InDisplayText);

	/**
	 * @brief Advances selection to the next option, wrapping around to the first.
	 * Called by UWidget_ListEntry_String::OnNextOptionButtonClicked.
	 */
	void AdvanceToNextOption();

	/**
	 * @brief Returns selection to the previous option, wrapping around to the last.
	 * Called by UWidget_ListEntry_String::OnPreviousOptionButtonClicked.
	 */
	void BackToPreviousOption();

	/**
	 * @brief Handles value changes initiated by the rotator widget via gamepad.
	 *
	 * Called by UWidget_ListEntry_String::OnRotatorValueChanged when the user
	 * rotates through options using a gamepad. Finds the matching string value
	 * from the selected display text and writes it via the dynamic setter.
	 *
	 * @param InNewSelectedText The display text of the newly selected option.
	 */
	void OnRotatorInitiatedValueChange(const FText& InNewSelectedText);

	/** @brief Returns the array of localized display texts for all available options. */
	FORCEINLINE const TArray<FText>& GetAvailableOptionsTextArray() const { return AvailableOptionsTextArray; }

	/** @brief Returns the localized display text of the currently selected option. */
	FORCEINLINE FText GetCurrentDisplayText() const { return CurrentDisplayText; }

protected:
	//~ Begin UListDataObject_Base Interface
	/**
	 * @brief Initializes the current value from the dynamic getter or default value.
	 * Called during InitDataObject — reads the active setting from GameUserSettings
	 * and synchronizes CurrentStringValue and CurrentDisplayText.
	 */
	virtual void OnDataObjectInitialized() override;

	/**
	 * @brief Returns true if the current string value differs from the default.
	 */
	virtual bool CanResetBackToDefaultValue() const override;

	/**
	 * @brief Resets to the default string value via the dynamic setter.
	 * @return True if the reset was successful.
	 */
	virtual bool TryResetBackToDefaultValue() override;

	/**
	 * @brief Returns true if the forced value exists in the available options array.
	 * Prevents forcing an invalid value that would leave the rotator in an inconsistent state.
	 */
	virtual bool CanSetToForcedStringValue(const FString& InForcedValue) const override;

	/**
	 * @brief Applies the forced string value and notifies listeners of a dependency-driven change.
	 */
	virtual void OnSetToForcedStringValue(const FString& InForcedValue) override;
	//~ End UListDataObject_Base Interface

	/**
	 * @brief Attempts to set CurrentDisplayText from an internal string value.
	 * Looks up the string value in AvailableOptionsStringArray and sets
	 * the corresponding display text from AvailableOptionsTextArray.
	 *
	 * @param InStringValue The internal string value to look up.
	 * @return True if a matching display text was found and set.
	 */
	bool TrySetDisplayTextFromStringValue(const FString& InStringValue);

	/** @brief The currently selected internal string value (written to GameUserSettings). */
	FString CurrentStringValue;

	/** @brief The localized display text corresponding to CurrentStringValue. */
	FText CurrentDisplayText;

	/** @brief Parallel array of internal string values for each available option. */
	TArray<FString> AvailableOptionsStringArray;

	/** @brief Parallel array of localized display texts for each available option. */
	TArray<FText> AvailableOptionsTextArray;
};

// =============================================================================
// SUBCLASSES
// =============================================================================

/**
 * @class UListDataObject_StringBool
 * @brief String data object specialized for boolean toggle settings.
 *
 * Registers "true" and "false" as the two available string options,
 * with customizable display texts (default: "ON" / "OFF").
 * Provides explicit SetTrueAsDefaultValue and SetFalseAsDefaultValue helpers
 * for cleaner registry code vs. calling SetDefaultValueFromString directly.
 */
UCLASS()
class FRONTENDUI_API UListDataObject_StringBool : public UListDataObject_String
{
	GENERATED_BODY()

public:
	/**
	 * @brief Overrides the display text for the "true" option.
	 * If "true" has not been added yet, adds it with the given text.
	 * @param InNewTrueDisplayText The display text to show when the setting is enabled.
	 */
	void OverrideTrueDisplayText(const FText& InNewTrueDisplayText);

	/**
	 * @brief Overrides the display text for the "false" option.
	 * If "false" has not been added yet, adds it with the given text.
	 * @param InNewFalseDisplayText The display text to show when the setting is disabled.
	 */
	void OverrideFalseDisplayText(const FText& InNewFalseDisplayText);

	/** @brief Sets "true" as the default value for reset support. */
	void SetTrueAsDefaultValue();

	/** @brief Sets "false" as the default value for reset support. */
	void SetFalseAsDefaultValue();

protected:
	//~ Begin UListDataObject_String Interface
	/**
	 * @brief Registers "true"/"false" options with default "ON"/"OFF" display texts
	 * before calling Super::OnDataObjectInitialized to read the current value.
	 */
	virtual void OnDataObjectInitialized() override;
	//~ End UListDataObject_String Interface

private:
	/** @brief Ensures "true" and "false" options are added if not already present. */
	void TryInitBoolValues();

	/** @brief Internal string representing the true/enabled state. */
	const FString TrueString = TEXT("true");

	/** @brief Internal string representing the false/disabled state. */
	const FString FalseString = TEXT("false");
};

/**
 * @class UListDataObject_StringEnum
 * @brief String data object specialized for enum-based settings.
 *
 * Provides template helpers for adding enum options and reading the current
 * value as a typed enum, avoiding manual string-to-enum conversion at call sites.
 */
UCLASS()
class FRONTENDUI_API UListDataObject_StringEnum : public UListDataObject_String
{
	GENERATED_BODY()

public:
	/**
	 * @brief Adds an enum value as a selectable option.
	 *
	 * Converts the enum value to its string representation via UEnum::GetNameStringByValue,
	 * then adds it alongside the provided display text.
	 *
	 * @tparam EnumType  The enum type (must be a UENUM).
	 * @param InEnumOption   The enum value to add.
	 * @param InDisplayText  Localized display text for this option.
	 */
	template <typename EnumType>
	void AddEnumOption(EnumType InEnumOption,
	                   const FText& InDisplayText)
	{
		const UEnum* StaticEnumOption = StaticEnum<EnumType>();
		const FString ConvertedEnumString = StaticEnumOption->GetNameStringByValue(static_cast<int64>(InEnumOption));

		AddDynamicOption(ConvertedEnumString,
		                 InDisplayText);
	}

	/**
	 * @brief Returns the current value as a typed enum.
	 *
	 * Converts CurrentStringValue back to the enum type via UEnum::GetValueByNameString.
	 *
	 * @tparam EnumType The enum type to cast to.
	 * @return The current value as EnumType.
	 */
	template <typename EnumType>
	EnumType GetCurrentValueAsEnum() const
	{
		const UEnum* StaticEnumOption = StaticEnum<EnumType>();

		return static_cast<EnumType>(StaticEnumOption->GetValueByNameString(CurrentStringValue));
	}

	/**
	 * @brief Sets the default value from an enum option for reset support.
	 *
	 * @tparam EnumType   The enum type.
	 * @param InEnumOption The enum value to use as the default.
	 */
	template <typename EnumType>
	void SetDefaultValueFromEnumOption(EnumType InEnumOption)
	{
		const UEnum* StaticEnumOption = StaticEnum<EnumType>();
		const FString ConvertedEnumString = StaticEnumOption->GetNameStringByValue(static_cast<int64>(InEnumOption));

		SetDefaultValueFromString(ConvertedEnumString);
	}
};

/**
 * @class UListDataObject_StringInteger
 * @brief String data object specialized for integer preset settings.
 *
 * Stores integer options as strings internally (e.g., "30", "60", "120" for frame rate presets)
 * while displaying them as FText. Handles the "Custom" fallback when the current
 * value doesn't match any registered preset.
 */
UCLASS()
class FRONTENDUI_API UListDataObject_StringInteger : public UListDataObject_String
{
	GENERATED_BODY()

public:
	/**
	 * @brief Adds an integer value as a selectable option.
	 * Converts InIntegerValue to string internally via LexToString.
	 *
	 * @param InIntegerValue The integer value (stored as string internally).
	 * @param InDisplayText  Localized display text for this option.
	 */
	void AddIntegerOption(int32 InIntegerValue,
	                      const FText& InDisplayText);

protected:
	//~ Begin UListDataObject_String Interface
	/**
	 * @brief After base initialization, sets CurrentDisplayText to "Custom"
	 * if the current value doesn't match any registered preset.
	 */
	virtual void OnDataObjectInitialized() override;

	/**
	 * @brief Updates CurrentStringValue from the getter when a dependency changes,
	 * setting display text to "Custom" if the new value is not a registered preset.
	 */
	virtual void OnEditDependencyDataModified(UListDataObject_Base* ModifiedDependencyData,
	                                          EOptionsListDataModifyReason ModifyReason) override;
	//~ End UListDataObject_String Interface
};
