#include "Widgets/Options/ListEntries/Widget_ListEntry_Scalar.h"

#include "Widgets/Options/DataObjects/ListDataObject_Scalar.h"
#include "AnalogSlider.h"

void UWidget_ListEntry_Scalar::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Bind slider delegates here (after widget initialization) rather than in
	// OnOwningListDataObjectSet, because Slate widgets must be fully constructed
	// before delegates can be safely bound.
	AnalogSlider_SettingSlider->OnValueChanged.AddUniqueDynamic(this,
	                                                            &ThisClass::OnSliderValueChanged);

	AnalogSlider_SettingSlider->OnMouseCaptureBegin.AddUniqueDynamic(this,
	                                                                 &ThisClass::OnSliderMouseCaptureBegin);
}

void UWidget_ListEntry_Scalar::OnOwningListDataObjectSet(UListDataObject_Base* InOwningListDataObject)
{
	Super::OnOwningListDataObjectSet(InOwningListDataObject);

	// Cache a typed pointer to avoid repeated CastChecked calls on every slider event.
	CachedOwningScalarDataObject = CastChecked<UListDataObject_Scalar>(InOwningListDataObject);

	// Configure the numeric text block with the scalar's display type and formatting.
	CommonNumeric_SettingValue->SetNumericType(CachedOwningScalarDataObject->GetDisplayNumericType());
	CommonNumeric_SettingValue->FormattingSpecification = CachedOwningScalarDataObject->GetNumberFormattingOptions();
	CommonNumeric_SettingValue->SetCurrentValue(CachedOwningScalarDataObject->GetCurrentValue());

	// Configure the slider with the scalar's display range and step size.
	// The slider operates in DisplayValueRange — mapping to OutputValueRange
	// is handled internally by UListDataObject_Scalar::SetCurrentValueFromSlider.
	AnalogSlider_SettingSlider->SetMinValue(CachedOwningScalarDataObject->GetDisplayValueRange().GetLowerBoundValue());
	AnalogSlider_SettingSlider->SetMaxValue(CachedOwningScalarDataObject->GetDisplayValueRange().GetUpperBoundValue());
	AnalogSlider_SettingSlider->SetStepSize(CachedOwningScalarDataObject->GetSliderStepSize());
	AnalogSlider_SettingSlider->SetValue(CachedOwningScalarDataObject->GetCurrentValue());
}

void UWidget_ListEntry_Scalar::OnOwningListDataObjectModified(UListDataObject_Base* OwningModifiedData,
                                                              EOptionsListDataModifyReason ModifyReason)
{
	if (CachedOwningScalarDataObject)
	{
		// Sync both the numeric text block and the slider position to the
		// data object's current display value. This handles resets and
		// dependency-driven value changes that occur outside user slider interaction.
		const float CurrentDisplayValue = CachedOwningScalarDataObject->GetCurrentValue();
		CommonNumeric_SettingValue->SetCurrentValue(CurrentDisplayValue);
		AnalogSlider_SettingSlider->SetValue(CurrentDisplayValue);
	}
}

void UWidget_ListEntry_Scalar::OnSliderValueChanged(float Value)
{
	if (CachedOwningScalarDataObject)
	{
		// Forward the slider's display-range value to the data object.
		// The data object maps it to OutputValueRange before writing to GameUserSettings.
		CachedOwningScalarDataObject->SetCurrentValueFromSlider(Value);
	}
}

void UWidget_ListEntry_Scalar::OnSliderMouseCaptureBegin()
{
	// Select this entry when the user starts dragging, ensuring the
	// options details view shows this setting's description during interaction.
	SelectThisEntryWidget();
}
