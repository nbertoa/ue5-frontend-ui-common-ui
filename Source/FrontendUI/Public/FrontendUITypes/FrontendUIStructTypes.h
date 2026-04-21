#pragma once

#include "CoreMinimal.h"
#include "FrontendUIStructTypes.generated.h"

/**
 * @brief Describes a conditional edit rule for a list data object.
 *
 * Used by UListDataObject_Base to determine whether a settings entry is currently
 * editable. Multiple descriptors can be added to a single data object — all
 * conditions must be met for the entry to be editable.
 *
 * When a condition is NOT met, this descriptor can optionally:
 * - Display a rich-text reason explaining why the entry is disabled.
 * - Force the entry to a specific string value (e.g., lock resolution when windowed).
 *
 * @see UListDataObject_Base::AddEditCondition
 * @see UOptionsDataRegistry
 */
USTRUCT()
struct FOptionsDataEditConditionDescriptor
{
	GENERATED_BODY()

public:
	/**
	 * @brief Sets the function that evaluates whether this condition is met.
	 *
	 * The function should return true if the associated setting is editable,
	 * false if it should be locked/disabled.
	 *
	 * @param InEditConditionFunc A callable (lambda or function) returning bool.
	 */
	void SetEditConditionFunc(TFunction<bool()> InEditConditionFunc)
	{
		EditConditionFunc = InEditConditionFunc;
	}

	/**
	 * @brief Returns true if a condition function has been assigned.
	 * An invalid descriptor (no function set) is treated as always-met.
	 */
	bool IsValid() const
	{
		return EditConditionFunc != nullptr;
	}

	/**
	 * @brief Evaluates the condition function.
	 * @return True if the condition is met (setting is editable), or true if no function is set.
	 */
	bool IsEditConditionMet() const
	{
		if (IsValid())
		{
			return EditConditionFunc();
		}

		// No condition function set — treat as always editable.
		return true;
	}

	/**
	 * @brief Returns the rich-text string explaining why the setting is disabled.
	 * Displayed in the options details view when the condition is not met.
	 */
	FString GetDisabledRichReason() const
	{
		return DisabledRichReason;
	}

	/**
	 * @brief Sets the rich-text reason shown when this condition blocks editing.
	 * @param InRichReason Rich-text formatted string (supports Common UI rich text tags).
	 */
	void SetDisabledRichReason(const FString& InRichReason)
	{
		DisabledRichReason = InRichReason;
	}

	/**
	 * @brief Returns true if this condition forces the data object to a specific string value when unmet.
	 * Example: When windowed mode is active, force resolution to the maximum allowed value.
	 */
	bool HasForcedStringValue() const
	{
		return DisabledForcedStringValue.IsSet();
	}

	/**
	 * @brief Returns the forced string value to apply when this condition is not met.
	 * @warning Only call this after confirming HasForcedStringValue() returns true.
	 */
	FString GetDisabledForcedStringValue() const
	{
		return DisabledForcedStringValue.GetValue();
	}

	/**
	 * @brief Sets the value that will be forced onto the data object when this condition is not met.
	 * @param InForcedValue The string value to apply (format must match the data object's value format).
	 */
	void SetDisabledForcedStringValue(const FString& InForcedValue)
	{
		DisabledForcedStringValue = InForcedValue;
	}

private:
	/** @brief The condition function. Returns true if the setting is editable. Null = always editable. */
	TFunction<bool()> EditConditionFunc;

	/** @brief Rich-text string shown in the details view when this condition blocks editing. */
	FString DisabledRichReason;

	/**
	 * @brief Optional value to force onto the data object when the condition is not met.
	 * Uses TOptional to distinguish between "no forced value" and "forced to empty string".
	 */
	TOptional<FString> DisabledForcedStringValue;
};
