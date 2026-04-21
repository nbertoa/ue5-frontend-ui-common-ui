#pragma once

#include "CoreMinimal.h"
#include "PropertyPathHelpers.h"

/**
 * @class FOptionsDataInteractionHelper
 * @brief Dynamic getter/setter bridge between settings data objects and UFrontendUIGameUserSettings.
 *
 * Provides a string-based interface for reading and writing UFrontendUIGameUserSettings
 * properties at runtime, without requiring compile-time knowledge of which property
 * is being accessed. This allows UOptionsDataRegistry to bind data objects to
 * settings properties using only function name strings, keeping the registry
 * decoupled from the specific properties of UFrontendUIGameUserSettings.
 *
 * @section Architecture Dynamic Property Access via FCachedPropertyPath
 * Uses Unreal's PropertyPathHelpers system (FCachedPropertyPath) to resolve
 * a function path string into a cached property accessor at construction time.
 * At runtime, GetValueAsString and SetValueFromString invoke the resolved
 * getter/setter via PropertyPathHelpers::GetPropertyValueAsString and
 * SetPropertyValueFromString, which call the actual UFUNCTION on the settings object.
 *
 * This pattern allows the options registry to register any GameUserSettings
 * property as a data object without writing per-property binding code:
 *
 * @code
 * // In UOptionsDataRegistry:
 * auto Getter = MakeShared<FOptionsDataInteractionHelper>("GetOverallVolume");
 * auto Setter = MakeShared<FOptionsDataInteractionHelper>("SetOverallVolume");
 * ScalarData->SetDataDynamicGetter(Getter);
 * ScalarData->SetDataDynamicSetter(Setter);
 * @endcode
 *
 * @section Ownership
 * Instances are owned by UListDataObject_Value via TSharedPtr, allowing multiple
 * data objects to share the same helper if they read/write the same property.
 *
 * @see UOptionsDataRegistry for where helpers are created and assigned.
 * @see UListDataObject_Value for the data objects that own these helpers.
 * @see UFrontendUIGameUserSettings for the settings properties accessed via helpers.
 */
class FRONTENDUI_API FOptionsDataInteractionHelper
{
public:
	/**
	 * @brief Constructs the helper and caches the property path for the given function name.
	 *
	 * The function path is resolved into an FCachedPropertyPath at construction time,
	 * avoiding repeated string parsing on every get/set call.
	 *
	 * @param InSetterOrGetterFuncPath The name of the getter or setter UFUNCTION on
	 *        UFrontendUIGameUserSettings (e.g., "GetOverallVolume", "SetOverallVolume").
	 */
	FOptionsDataInteractionHelper(const FString& InSetterOrGetterFuncPath);

	/**
	 * @brief Reads the current value of the bound property as a string.
	 *
	 * Invokes the getter UFUNCTION on UFrontendUIGameUserSettings via
	 * PropertyPathHelpers::GetPropertyValueAsString and returns the result.
	 *
	 * @return The current property value as a string (e.g., "0.75" for a float volume).
	 *         Returns an empty string if the property path is invalid or the settings are null.
	 */
	FString GetValueAsString() const;

	/**
	 * @brief Writes a new value to the bound property from a string.
	 *
	 * Invokes the setter UFUNCTION on UFrontendUIGameUserSettings via
	 * PropertyPathHelpers::SetPropertyValueFromString, converting the string
	 * to the appropriate type automatically.
	 *
	 * @param InStringValue The new value as a string (e.g., "0.75" for a float volume).
	 */
	void SetValueFromString(const FString& InStringValue) const;

private:
	/**
	 * @brief Cached property path resolved from the function name string at construction.
	 * FCachedPropertyPath avoids repeated string parsing on every get/set call,
	 * making repeated access efficient even in per-frame or per-input scenarios.
	 */
	FCachedPropertyPath CachedDynamicFunctionPath;

	/**
	 * @brief Weak reference to the active UFrontendUIGameUserSettings instance.
	 * TWeakObjectPtr prevents this helper from keeping the settings object alive
	 * beyond its normal lifetime, allowing safe use across level transitions.
	 */
	TWeakObjectPtr<class UFrontendUIGameUserSettings> CachedWeakGameUserSettings;
};
