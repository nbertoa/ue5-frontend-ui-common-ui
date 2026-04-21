#include "Widgets/Options/DataObjects/ListDataObject_String.h"

#include "Widgets/Options/OptionsDataInteractionHelper.h"

// =============================================================================
// UListDataObject_String
// =============================================================================

void UListDataObject_String::OnDataObjectInitialized()
{
	// Priority order for initial value resolution:
	// 1. First available option (index 0) — ensures a valid starting point.
	// 2. Default value (if set) — overrides the first option if a default is configured.
	// 3. Dynamic getter (current GameUserSettings value) — overrides everything if available.
	if (!AvailableOptionsStringArray.IsEmpty())
	{
		CurrentStringValue = AvailableOptionsStringArray[0];
	}

	if (HasDefaultValue())
	{
		CurrentStringValue = GetDefaultValueAsString();
	}

	if (DataDynamicGetter)
	{
		if (!DataDynamicGetter->GetValueAsString().IsEmpty())
		{
			CurrentStringValue = DataDynamicGetter->GetValueAsString();
		}
	}

	// Sync the display text to match the resolved string value.
	// If no match is found (e.g., an invalid saved value), fall back to "Invalid Option".
	if (!TrySetDisplayTextFromStringValue(CurrentStringValue))
	{
		CurrentDisplayText = FText::FromString(TEXT("Invalid Option"));
	}
}

void UListDataObject_String::AddDynamicOption(const FString& InStringValue,
                                              const FText& InDisplayText)
{
	AvailableOptionsStringArray.Add(InStringValue);
	AvailableOptionsTextArray.Add(InDisplayText);
}

void UListDataObject_String::AdvanceToNextOption()
{
	if (AvailableOptionsStringArray.IsEmpty() || AvailableOptionsTextArray.IsEmpty())
	{
		return;
	}

	const int32 CurrentDisplayIndex = AvailableOptionsStringArray.IndexOfByKey(CurrentStringValue);
	const int32 NextIndexToDisplay = CurrentDisplayIndex + 1;

	// Wrap around to the first option when reaching the end.
	CurrentStringValue = AvailableOptionsStringArray.IsValidIndex(NextIndexToDisplay)
		                     ? AvailableOptionsStringArray[NextIndexToDisplay]
		                     : AvailableOptionsStringArray[0];

	TrySetDisplayTextFromStringValue(CurrentStringValue);

	if (DataDynamicSetter)
	{
		DataDynamicSetter->SetValueFromString(CurrentStringValue);
		NotifyListDataModified(this);
	}
}

void UListDataObject_String::BackToPreviousOption()
{
	if (AvailableOptionsStringArray.IsEmpty() || AvailableOptionsTextArray.IsEmpty())
	{
		return;
	}

	const int32 CurrentDisplayIndex = AvailableOptionsStringArray.IndexOfByKey(CurrentStringValue);
	const int32 PreviousIndexToDisplay = CurrentDisplayIndex - 1;

	// Wrap around to the last option when going before the first.
	CurrentStringValue = AvailableOptionsStringArray.IsValidIndex(PreviousIndexToDisplay)
		                     ? AvailableOptionsStringArray[PreviousIndexToDisplay]
		                     : AvailableOptionsStringArray.Last();

	TrySetDisplayTextFromStringValue(CurrentStringValue);

	if (DataDynamicSetter)
	{
		DataDynamicSetter->SetValueFromString(CurrentStringValue);
		NotifyListDataModified(this);
	}
}

void UListDataObject_String::OnRotatorInitiatedValueChange(const FText& InNewSelectedText)
{
	// Find the string value matching the display text selected by the rotator.
	// This bridges the rotator's index-based selection back to the string-based data model.
	const int32 FoundIndex = AvailableOptionsTextArray.IndexOfByPredicate(
		[InNewSelectedText](const FText& AvailableText) -> bool
		{
			return AvailableText.EqualTo(InNewSelectedText);
		});

	if (FoundIndex != INDEX_NONE && AvailableOptionsStringArray.IsValidIndex(FoundIndex))
	{
		CurrentDisplayText = InNewSelectedText;
		CurrentStringValue = AvailableOptionsStringArray[FoundIndex];

		if (DataDynamicSetter)
		{
			DataDynamicSetter->SetValueFromString(CurrentStringValue);
			NotifyListDataModified(this);
		}
	}
}

bool UListDataObject_String::CanResetBackToDefaultValue() const
{
	return HasDefaultValue() && CurrentStringValue != GetDefaultValueAsString();
}

bool UListDataObject_String::TryResetBackToDefaultValue()
{
	if (CanResetBackToDefaultValue())
	{
		CurrentStringValue = GetDefaultValueAsString();
		TrySetDisplayTextFromStringValue(CurrentStringValue);

		if (DataDynamicSetter)
		{
			DataDynamicSetter->SetValueFromString(CurrentStringValue);
			NotifyListDataModified(this,
			                       EOptionsListDataModifyReason::ResetToDefault);

			return true;
		}
	}

	return false;
}

bool UListDataObject_String::CanSetToForcedStringValue(const FString& InForcedValue) const
{
	// Only allow forcing to values that exist in the options array.
	// Prevents forcing an invalid value that would leave the rotator in an inconsistent state.
	return CurrentStringValue != InForcedValue;
}

void UListDataObject_String::OnSetToForcedStringValue(const FString& InForcedValue)
{
	CurrentStringValue = InForcedValue;
	TrySetDisplayTextFromStringValue(CurrentStringValue);

	if (DataDynamicSetter)
	{
		DataDynamicSetter->SetValueFromString(CurrentStringValue);

		// Use DependencyModified reason — this change was forced by an external condition,
		// not by direct user interaction.
		NotifyListDataModified(this,
		                       EOptionsListDataModifyReason::DependencyModified);
	}
}

bool UListDataObject_String::TrySetDisplayTextFromStringValue(const FString& InStringValue)
{
	const int32 CurrentFoundIndex = AvailableOptionsStringArray.IndexOfByKey(InStringValue);

	if (AvailableOptionsTextArray.IsValidIndex(CurrentFoundIndex))
	{
		CurrentDisplayText = AvailableOptionsTextArray[CurrentFoundIndex];
		return true;
	}

	return false;
}

// =============================================================================
// UListDataObject_StringBool
// =============================================================================

void UListDataObject_StringBool::OverrideTrueDisplayText(const FText& InNewTrueDisplayText)
{
	// Add the true option if not already present — avoids duplicates.
	if (!AvailableOptionsStringArray.Contains(TrueString))
	{
		AddDynamicOption(TrueString,
		                 InNewTrueDisplayText);
	}
}

void UListDataObject_StringBool::OverrideFalseDisplayText(const FText& InNewFalseDisplayText)
{
	if (!AvailableOptionsStringArray.Contains(FalseString))
	{
		AddDynamicOption(FalseString,
		                 InNewFalseDisplayText);
	}
}

void UListDataObject_StringBool::SetTrueAsDefaultValue()
{
	SetDefaultValueFromString(TrueString);
}

void UListDataObject_StringBool::SetFalseAsDefaultValue()
{
	SetDefaultValueFromString(FalseString);
}

void UListDataObject_StringBool::OnDataObjectInitialized()
{
	// Register default bool options before calling Super,
	// which reads the current value and needs the options array populated first.
	TryInitBoolValues();

	Super::OnDataObjectInitialized();
}

void UListDataObject_StringBool::TryInitBoolValues()
{
	// Add default "ON"/"OFF" display texts if not already overridden.
	// OverrideTrueDisplayText/OverrideFalseDisplayText allow customization before InitDataObject.
	if (!AvailableOptionsStringArray.Contains(TrueString))
	{
		AddDynamicOption(TrueString,
		                 FText::FromString(TEXT("ON")));
	}

	if (!AvailableOptionsStringArray.Contains(FalseString))
	{
		AddDynamicOption(FalseString,
		                 FText::FromString(TEXT("OFF")));
	}
}

// =============================================================================
// UListDataObject_StringInteger
// =============================================================================

void UListDataObject_StringInteger::AddIntegerOption(int32 InIntegerValue,
                                                     const FText& InDisplayText)
{
	// Convert the integer to string via LexToString for consistent serialization
	// with the dynamic getter/setter string format.
	AddDynamicOption(LexToString(InIntegerValue),
	                 InDisplayText);
}

void UListDataObject_StringInteger::OnDataObjectInitialized()
{
	Super::OnDataObjectInitialized();

	// If the current value doesn't match any registered preset (e.g., a custom
	// frame rate set outside the options screen), display "Custom" instead of
	// "Invalid Option" — a more user-friendly fallback for integer presets.
	if (!TrySetDisplayTextFromStringValue(CurrentStringValue))
	{
		CurrentDisplayText = FText::FromString(TEXT("Custom"));
	}
}

void UListDataObject_StringInteger::OnEditDependencyDataModified(UListDataObject_Base* ModifiedDependencyData,
                                                                 EOptionsListDataModifyReason ModifyReason)
{
	if (DataDynamicGetter)
	{
		// Skip update if the value hasn't actually changed.
		if (CurrentStringValue == DataDynamicGetter->GetValueAsString())
		{
			return;
		}

		CurrentStringValue = DataDynamicGetter->GetValueAsString();

		// If the new value doesn't match a preset, show "Custom".
		if (!TrySetDisplayTextFromStringValue(CurrentStringValue))
		{
			CurrentDisplayText = FText::FromString(TEXT("Custom"));
		}

		NotifyListDataModified(this,
		                       EOptionsListDataModifyReason::DependencyModified);
	}

	Super::OnEditDependencyDataModified(ModifiedDependencyData,
	                                    ModifyReason);
}
