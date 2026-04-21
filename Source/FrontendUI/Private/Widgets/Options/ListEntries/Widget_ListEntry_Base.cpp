#include "Widgets/Options/ListEntries/Widget_ListEntry_Base.h"

#include "CommonTextBlock.h"
#include "Components/ListView.h"
#include "Widgets/Options/DataObjects/ListDataObject_Base.h"
#include "CommonInputSubsystem.h"

void UWidget_ListEntry_Base::NativeOnListEntryWidgetHovered(bool bWasHovered)
{
	// Resolve selection state at the moment of hover — the entry may or may not
	// be selected when hovered, and Blueprint uses both states for visual feedback.
	BP_OnListEntryWidgetHovered(bWasHovered,
	                            GetListItem()
		                            ? IsListItemSelected()
		                            : false);
}

void UWidget_ListEntry_Base::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	// CastChecked: UFrontendUICommonListView only feeds UListDataObject_Base objects
	// to this widget — a non-matching type indicates a configuration error.
	OnOwningListDataObjectSet(CastChecked<UListDataObject_Base>(ListItemObject));
}

void UWidget_ListEntry_Base::NativeOnEntryReleased()
{
	IUserObjectListEntry::NativeOnEntryReleased();

	// Clear hover state when the entry is recycled by the list view.
	// Without this, a recycled entry might display stale hover visuals
	// when reused for a different data object.
	NativeOnListEntryWidgetHovered(false);
}

FReply UWidget_ListEntry_Base::NativeOnFocusReceived(const FGeometry& InGeometry,
                                                     const FFocusEvent& InFocusEvent)
{
	UCommonInputSubsystem* CommonInputSubsystem = GetInputSubsystem();

	// Only redirect focus for gamepad input — keyboard and mouse use default UMG focus behavior.
	if (CommonInputSubsystem && CommonInputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad)
	{
		if (UWidget* WidgetToFocus = BP_GetWidgetToFocusForGamepad())
		{
			if (TSharedPtr<SWidget> SlateWidgetToFocus = WidgetToFocus->GetCachedWidget())
			{
				// SetUserFocus redirects Slate focus to the designated widget,
				// ensuring gamepad input goes to the correct interactive control.
				return FReply::Handled().SetUserFocus(SlateWidgetToFocus.ToSharedRef());
			}
		}
	}

	return Super::NativeOnFocusReceived(InGeometry,
	                                    InFocusEvent);
}

void UWidget_ListEntry_Base::OnOwningListDataObjectSet(UListDataObject_Base* InOwningListDataObject)
{
	// Update the display name text block if bound.
	if (CommonText_SettingDisplayName)
	{
		CommonText_SettingDisplayName->SetText(InOwningListDataObject->GetDataDisplayName());
	}

	// Bind to OnListDataModified to receive future value change notifications.
	// IsBoundToObject guard prevents duplicate bindings when the list view
	// recycles this entry widget for a different data object of the same type.
	if (!InOwningListDataObject->OnListDataModified.IsBoundToObject(this))
	{
		InOwningListDataObject->OnListDataModified.AddUObject(this,
		                                                      &ThisClass::OnOwningListDataObjectModified);
	}

	// Bind to OnDependencyDataModified to re-evaluate editable state
	// when a dependency setting changes (e.g., window mode affects resolution).
	if (!InOwningListDataObject->OnDependencyDataModified.IsBoundToObject(this))
	{
		InOwningListDataObject->OnDependencyDataModified.AddUObject(this,
		                                                            &ThisClass::OnOwningDependencyDataObjectModified);
	}

	// Set initial editable state based on current edit conditions.
	OnToggleEditableState(InOwningListDataObject->IsDataCurrentlyEditable());

	CachedOwningDataObject = InOwningListDataObject;
}

void UWidget_ListEntry_Base::OnOwningListDataObjectModified(UListDataObject_Base* OwningModifiedData,
                                                            EOptionsListDataModifyReason ModifyReason)
{
	// Base implementation is intentionally empty.
	// Subclasses override to update their specific interactive controls.
}

void UWidget_ListEntry_Base::OnToggleEditableState(bool bIsEditable)
{
	// Base implementation only affects the display name text block.
	// Subclasses call Super and then disable their specific controls
	// (slider, rotator, navigation buttons).
	if (CommonText_SettingDisplayName)
	{
		CommonText_SettingDisplayName->SetIsEnabled(bIsEditable);
	}
}

void UWidget_ListEntry_Base::SelectThisEntryWidget() const
{
	// Cast to UListView (not UCommonListView) for API access to SetSelectedItem.
	// The owning list view is guaranteed to be a UListView subclass in this context.
	CastChecked<UListView>(GetOwningListView())->SetSelectedItem(GetListItem());
}

void UWidget_ListEntry_Base::OnOwningDependencyDataObjectModified(UListDataObject_Base* OwningModifiedDependencyData,
                                                                  EOptionsListDataModifyReason ModifyReason)
{
	// Re-evaluate the editable state when a dependency changes.
	// IsDataCurrentlyEditable also handles forced value application,
	// so this call may trigger OnSetToForcedStringValue on the data object.
	if (CachedOwningDataObject)
	{
		OnToggleEditableState(CachedOwningDataObject->IsDataCurrentlyEditable());
	}
}
