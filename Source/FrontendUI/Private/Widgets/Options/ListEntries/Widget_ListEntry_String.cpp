#include "Widgets/Options/ListEntries/Widget_ListEntry_String.h"

#include "Widgets/Options/DataObjects/ListDataObject_String.h"
#include "Widgets/Components/FrontendUICommonRotator.h"
#include "Widgets/Components/FrontendUICommonButtonBase.h"
#include "CommonInputSubsystem.h"

void UWidget_ListEntry_String::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Bind previous/next button click delegates.
	CommonButton_PreviousOption->OnClicked().AddUObject(this,
	                                                    &ThisClass::OnPreviousOptionButtonClicked);
	CommonButton_NextOption->OnClicked().AddUObject(this,
	                                                &ThisClass::OnNextOptionButtonClicked);

	// Bind rotator click to selection only — clicking the rotator selects
	// this entry in the list without changing the value (value changes come
	// from the previous/next buttons or gamepad rotation).
	CommonRotator_AvailableOptions->OnClicked().AddLambda([this]() { SelectThisEntryWidget(); });

	// Bind rotator rotation event for gamepad input handling.
	CommonRotator_AvailableOptions->OnRotatedEvent.AddUObject(this,
	                                                          &ThisClass::OnRotatorValueChanged);
}

void UWidget_ListEntry_String::OnOwningListDataObjectSet(UListDataObject_Base* InOwningListDataObject)
{
	Super::OnOwningListDataObjectSet(InOwningListDataObject);

	// Cache a typed pointer to avoid repeated CastChecked calls on every button event.
	CachedOwningStringDataObject = CastChecked<UListDataObject_String>(InOwningListDataObject);

	// Populate the rotator's text label array from the data object's available options.
	CommonRotator_AvailableOptions->PopulateTextLabels(CachedOwningStringDataObject->GetAvailableOptionsTextArray());

	// Set the rotator's initial selection to match the data object's current value.
	CommonRotator_AvailableOptions->SetSelectedOptionByText(CachedOwningStringDataObject->GetCurrentDisplayText());
}

void UWidget_ListEntry_String::OnOwningListDataObjectModified(UListDataObject_Base* OwningModifiedData,
                                                              EOptionsListDataModifyReason ModifyReason)
{
	if (CachedOwningStringDataObject)
	{
		// Sync the rotator's displayed option to the data object's current value.
		// This handles resets and dependency-driven forced value changes
		// that occur outside of direct user button interaction.
		CommonRotator_AvailableOptions->SetSelectedOptionByText(CachedOwningStringDataObject->GetCurrentDisplayText());
	}
}

void UWidget_ListEntry_String::OnToggleEditableState(bool bIsEditable)
{
	// Call Super to disable the display name text block first.
	Super::OnToggleEditableState(bIsEditable);

	// Disable all three interactive controls when the setting is locked.
	CommonButton_PreviousOption->SetIsEnabled(bIsEditable);
	CommonRotator_AvailableOptions->SetIsEnabled(bIsEditable);
	CommonButton_NextOption->SetIsEnabled(bIsEditable);
}

void UWidget_ListEntry_String::OnPreviousOptionButtonClicked()
{
	if (CachedOwningStringDataObject)
	{
		CachedOwningStringDataObject->BackToPreviousOption();
	}

	// Select this entry to ensure the details view shows this setting's description.
	SelectThisEntryWidget();
}

void UWidget_ListEntry_String::OnNextOptionButtonClicked()
{
	if (CachedOwningStringDataObject)
	{
		CachedOwningStringDataObject->AdvanceToNextOption();
	}

	SelectThisEntryWidget();
}

void UWidget_ListEntry_String::OnRotatorValueChanged(int32 Value,
                                                     bool bUserInitiated) const
{
	if (!CachedOwningStringDataObject)
	{
		return;
	}

	UCommonInputSubsystem* CommonInputSubsystem = GetInputSubsystem();
	if (!CommonInputSubsystem || !bUserInitiated)
	{
		return;
	}

	// Only process gamepad-initiated rotator changes here.
	// Mouse-initiated value changes are handled by the previous/next buttons.
	// The bUserInitiated guard prevents this from firing during programmatic
	// SetSelectedOptionByText calls in OnOwningListDataObjectModified.
	if (CommonInputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad)
	{
		CachedOwningStringDataObject->OnRotatorInitiatedValueChange(CommonRotator_AvailableOptions->GetSelectedText());
	}
}
