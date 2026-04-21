#pragma once

#include "CoreMinimal.h"
#include "Widgets/Options/DataObjects/ListDataObject_Base.h"
#include "ListDataObject_Value.generated.h"

/**
 * @class UListDataObject_Value
 * @brief Abstract base class for data objects that read and write a settings value.
 *
 * Extends UListDataObject_Base with dynamic getter/setter binding via
 * FOptionsDataInteractionHelper, and an optional default value for reset support.
 *
 * @section Architecture Dynamic Getter/Setter Pattern
 * Rather than hardcoding which GameUserSettings property each data object accesses,
 * UListDataObject_Value stores TSharedPtr references to FOptionsDataInteractionHelper
 * instances. Each helper encapsulates a single UFUNCTION path (e.g., "GetOverallVolume")
 * and uses PropertyPathHelpers to invoke it dynamically at runtime.
 *
 * This allows UOptionsDataRegistry to bind any GameUserSettings property to any
 * data object type without writing per-property binding code. The data object
 * does not need to know which property it is bound to — it just calls
 * DataDynamicGetter->GetValueAsString() and DataDynamicSetter->SetValueFromString().
 *
 * @section Default Value and Reset
 * An optional TOptional<FString> default value enables the reset system.
 * When set, CanResetBackToDefaultValue() compares the current value against
 * the default, and TryResetBackToDefaultValue() restores it.
 * UWidget_OptionsScreen uses HasDefaultValue() to decide whether to show
 * the Reset action binding for the current tab.
 *
 * @see FOptionsDataInteractionHelper for the dynamic property access implementation.
 * @see UOptionsDataRegistry for where getters/setters are created and assigned.
 * @see UListDataObject_Scalar and UListDataObject_String for concrete subclasses.
 */
UCLASS(Abstract)
class FRONTENDUI_API UListDataObject_Value : public UListDataObject_Base
{
	GENERATED_BODY()

public:
	/**
	 * @brief Assigns the dynamic getter for this data object.
	 *
	 * The getter is called whenever the current value needs to be read
	 * from UFrontendUIGameUserSettings (e.g., on initialization or reset).
	 *
	 * @param InDynamicGetter Shared pointer to a helper bound to a getter UFUNCTION.
	 */
	void SetDataDynamicGetter(const TSharedPtr<class FOptionsDataInteractionHelper>& InDynamicGetter);

	/**
	 * @brief Assigns the dynamic setter for this data object.
	 *
	 * The setter is called whenever the user changes the value in the UI,
	 * writing it back to UFrontendUIGameUserSettings.
	 *
	 * @param InDynamicSetter Shared pointer to a helper bound to a setter UFUNCTION.
	 */
	void SetDataDynamicSetter(const TSharedPtr<class FOptionsDataInteractionHelper>& InDynamicSetter);

	/**
	 * @brief Sets the default value for reset support.
	 *
	 * Once set, HasDefaultValue() returns true and CanResetBackToDefaultValue()
	 * becomes meaningful. The default is stored as a string for type-agnostic
	 * comparison against the getter's current value.
	 *
	 * @param InDefaultValue The default value string (must match the getter's output format).
	 */
	void SetDefaultValueFromString(const FString& InDefaultValue) { DefaultStringValue = InDefaultValue; }

	//~ Begin UListDataObject_Base Interface
	/** @brief Returns true if a default value has been set via SetDefaultValueFromString. */
	virtual bool HasDefaultValue() const override { return DefaultStringValue.IsSet(); }
	//~ End UListDataObject_Base Interface

protected:
	/**
	 * @brief Returns the default value string.
	 * @warning Only call after confirming HasDefaultValue() returns true.
	 */
	FString GetDefaultValueAsString() const { return DefaultStringValue.GetValue(); }

	/**
	 * @brief Dynamic getter bound to a UFrontendUIGameUserSettings getter UFUNCTION.
	 * Null if no getter has been assigned — subclasses must guard before calling.
	 */
	TSharedPtr<class FOptionsDataInteractionHelper> DataDynamicGetter;

	/**
	 * @brief Dynamic setter bound to a UFrontendUIGameUserSettings setter UFUNCTION.
	 * Null if no setter has been assigned — subclasses must guard before calling.
	 */
	TSharedPtr<class FOptionsDataInteractionHelper> DataDynamicSetter;

private:
	/**
	 * @brief Optional default value for reset support.
	 *
	 * Uses TOptional to cleanly distinguish between "no default set" and
	 * "default is explicitly set to an empty string". IsSet() returns false
	 * until SetDefaultValueFromString is called.
	 */
	TOptional<FString> DefaultStringValue;
};
