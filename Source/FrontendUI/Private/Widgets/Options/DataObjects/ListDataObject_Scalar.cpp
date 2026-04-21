#include "Widgets/Options/DataObjects/ListDataObject_Scalar.h"

#include "Widgets/Options/OptionsDataInteractionHelper.h"

FCommonNumberFormattingOptions UListDataObject_Scalar::NoDecimal()
{
	FCommonNumberFormattingOptions Options;
	Options.MaximumFractionalDigits = 0;

	return Options;
}

FCommonNumberFormattingOptions UListDataObject_Scalar::WithDecimal(int32 NumFracDigit)
{
	FCommonNumberFormattingOptions Options;
	Options.MaximumFractionalDigits = NumFracDigit;

	return Options;
}

float UListDataObject_Scalar::GetCurrentValue() const
{
	if (DataDynamicGetter)
	{
		// Read the raw value from GameUserSettings (in OutputValueRange),
		// then map it to DisplayValueRange so the slider shows user-friendly values.
		// Example: 0.75 (output) → 75.0 (display, for a 0–100% slider).
		return FMath::GetMappedRangeValueClamped(OutputValueRange,
		                                         DisplayValueRange,
		                                         StringToFloat(DataDynamicGetter->GetValueAsString()));
	}

	return 0.0f;
}

void UListDataObject_Scalar::SetCurrentValueFromSlider(float InNewValue)
{
	if (DataDynamicSetter)
	{
		// Map from DisplayValueRange back to OutputValueRange before writing to settings.
		// Example: 75.0 (display) → 0.75 (output, written to GameUserSettings).
		const float ClampedValue = FMath::GetMappedRangeValueClamped(DisplayValueRange,
		                                                             OutputValueRange,
		                                                             InNewValue);

		DataDynamicSetter->SetValueFromString(LexToString(ClampedValue));

		// Notify listeners (entry widget, options screen) that the value changed.
		NotifyListDataModified(this);
	}
}

bool UListDataObject_Scalar::CanResetBackToDefaultValue() const
{
	if (HasDefaultValue() && DataDynamicGetter)
	{
		const float DefaultValue = StringToFloat(GetDefaultValueAsString());
		const float CurrentValue = StringToFloat(DataDynamicGetter->GetValueAsString());

		// Use a near-equality threshold of 0.01 to avoid floating point precision issues
		// causing spurious "needs reset" states for values that are effectively equal.
		return !FMath::IsNearlyEqual(DefaultValue,
		                             CurrentValue,
		                             0.01f);
	}

	return false;
}

bool UListDataObject_Scalar::TryResetBackToDefaultValue()
{
	if (CanResetBackToDefaultValue())
	{
		if (DataDynamicSetter)
		{
			DataDynamicSetter->SetValueFromString(GetDefaultValueAsString());

			NotifyListDataModified(this,
			                       EOptionsListDataModifyReason::ResetToDefault);

			return true;
		}
	}

	return false;
}

void UListDataObject_Scalar::OnEditDependencyDataModified(UListDataObject_Base* ModifiedDependencyData,
                                                          EOptionsListDataModifyReason ModifyReason)
{
	// Notify the slider widget to re-read and display the updated value.
	// This handles cases where a dependency change affects this scalar's
	// effective value (e.g., a resolution scale reset triggered by a quality preset change).
	NotifyListDataModified(this,
	                       EOptionsListDataModifyReason::DependencyModified);

	Super::OnEditDependencyDataModified(ModifiedDependencyData,
	                                    ModifyReason);
}

float UListDataObject_Scalar::StringToFloat(const FString& InString)
{
	float OutConvertedValue = 0.0f;

	// LexFromString handles the string-to-float conversion using Unreal's
	// built-in lexical casting — consistent with how LexToString serialized it.
	LexFromString(OutConvertedValue,
	              *InString);

	return OutConvertedValue;
}
