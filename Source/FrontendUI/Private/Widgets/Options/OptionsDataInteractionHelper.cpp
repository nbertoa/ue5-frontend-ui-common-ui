#include "Widgets/Options/OptionsDataInteractionHelper.h"

#include "FrontendUISettings/FrontendUIGameUserSettings.h"

FOptionsDataInteractionHelper::FOptionsDataInteractionHelper(const FString& InSetterOrGetterFuncPath)
	: CachedDynamicFunctionPath(InSetterOrGetterFuncPath)
{
	// Cache the GameUserSettings instance at construction time.
	// This avoids calling UFrontendUIGameUserSettings::Get() on every get/set operation,
	// since the settings object persists for the lifetime of the game session.
	CachedWeakGameUserSettings = UFrontendUIGameUserSettings::Get();
}

FString FOptionsDataInteractionHelper::GetValueAsString() const
{
	FString OutStringValue;

	// Invoke the getter UFUNCTION on the settings object via the cached property path.
	// PropertyPathHelpers handles type conversion from the native type (float, bool, FString)
	// to a string representation automatically.
	PropertyPathHelpers::GetPropertyValueAsString(CachedWeakGameUserSettings.Get(),
	                                              CachedDynamicFunctionPath,
	                                              OutStringValue);

	return OutStringValue;
}

void FOptionsDataInteractionHelper::SetValueFromString(const FString& InStringValue) const
{
	// Invoke the setter UFUNCTION on the settings object via the cached property path.
	// PropertyPathHelpers handles type conversion from the string representation
	// back to the native type (float, bool, FString) automatically.
	PropertyPathHelpers::SetPropertyValueFromString(CachedWeakGameUserSettings.Get(),
	                                                CachedDynamicFunctionPath,
	                                                InStringValue);
}
