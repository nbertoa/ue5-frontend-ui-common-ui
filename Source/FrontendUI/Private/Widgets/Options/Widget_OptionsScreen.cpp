#include "Widgets/Options/Widget_OptionsScreen.h"

#include "FrontendUISettings/FrontendUIGameUserSettings.h"
#include "ICommonInputModule.h"
#include "Input/CommonUIInputTypes.h"
#include "Widgets/Components/FrontendUICommonListView.h"
#include "Widgets/Components/FrontendUITabListWidgetBase.h"
#include "Widgets/Options/DataObjects/ListDataObject_Collection.h"
#include "Widgets/Options/ListEntries/Widget_ListEntry_Base.h"
#include "Widgets/Options/OptionsDataRegistry.h"
#include "Widgets/Options/Widget_OptionsDetailsView.h"
#include "Subsystems/FrontendUISubsystem.h"
#include "Widgets/Components/FrontendUICommonButtonBase.h"

void UWidget_OptionsScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// -------------------------------------------------------------------------
	// RESET ACTION BINDING
	// -------------------------------------------------------------------------
	// Register the Reset action handle but do NOT add it to active bindings yet.
	// It will be added dynamically when resettable data exists in the current tab.
	if (!ResetAction.IsNull())
	{
		FBindUIActionArgs BindActionArgs(ResetAction,
		                                 true,
		                                 FSimpleDelegate::CreateUObject(this,
		                                                                &ThisClass::OnResetBoundActionTriggered));

		ResetActionHandle = RegisterUIActionBinding(BindActionArgs);
	}

	// -------------------------------------------------------------------------
	// BACK ACTION BINDING
	// -------------------------------------------------------------------------
	// Always active — Back closes the options screen.
	FBindUIActionArgs BindActionArgs(ICommonInputModule::GetSettings().GetDefaultBackAction(),
	                                 true,
	                                 FSimpleDelegate::CreateUObject(this,
	                                                                &ThisClass::OnBackBoundActionTriggered));
	RegisterUIActionBinding(BindActionArgs);

	// -------------------------------------------------------------------------
	// DELEGATE BINDING
	// -------------------------------------------------------------------------
	TabListWidget_OptionsTabs->OnTabSelected.AddUniqueDynamic(this,
	                                                          &ThisClass::OnOptionsTabSelected);

	CommonListView_OptionsList->OnItemIsHoveredChanged().AddUObject(this,
	                                                                &ThisClass::OnListViewItemHovered);

	CommonListView_OptionsList->OnItemSelectionChanged().AddUObject(this,
	                                                                &ThisClass::OnListViewItemSelected);
}

void UWidget_OptionsScreen::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Register tabs from the data registry.
	// Guard against duplicate registration — NativeOnActivated fires every time
	// the widget is returned to (e.g., after closing a sub-screen), so tabs
	// from a previous activation must not be registered again.
	const TArray<UListDataObject_Collection*>& RegisteredOptionsTabCollections = GetOrCreateDataRegistry()->
			GetRegisteredOptionsTabCollections();

	for (UListDataObject_Collection* TabCollection : RegisteredOptionsTabCollections)
	{
		if (!TabCollection)
		{
			continue;
		}

		const FName TabID = TabCollection->GetDataID();
		const FText TabDisplayName = TabCollection->GetDataDisplayName();

		// Skip if this tab has already been registered from a previous activation.
		if (TabListWidget_OptionsTabs->GetTabButtonBaseByID(TabID) != nullptr)
		{
			continue;
		}

		TabListWidget_OptionsTabs->RequestRegisterTab(TabID,
		                                              TabDisplayName);
	}
}

void UWidget_OptionsScreen::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// Batch-apply all settings changes made during this session.
	// bMarkDirty=true ensures changes are persisted to GameUserSettings.ini.
	UFrontendUIGameUserSettings::Get()->ApplySettings(true);
}

UWidget* UWidget_OptionsScreen::NativeGetDesiredFocusTarget() const
{
	// Return the currently selected entry widget as the focus target,
	// so focus is restored to the correct list item when the screen is re-activated.
	if (UObject* SelectedObject = CommonListView_OptionsList->GetSelectedItem())
	{
		if (UUserWidget* SelectedEntryWidget = CommonListView_OptionsList->GetEntryWidgetFromItem(SelectedObject))
		{
			return SelectedEntryWidget;
		}
	}

	return Super::NativeGetDesiredFocusTarget();
}

UOptionsDataRegistry* UWidget_OptionsScreen::GetOrCreateDataRegistry()
{
	if (!CreatedOwningDataRegistry)
	{
		// Create the registry lazily on first access to avoid building the entire
		// options data hierarchy before the screen is first opened.
		CreatedOwningDataRegistry = NewObject<UOptionsDataRegistry>();
		CreatedOwningDataRegistry->InitOptionsDataRegistry(GetOwningLocalPlayer());
	}

	checkf(CreatedOwningDataRegistry,
	       TEXT("UWidget_OptionsScreen::GetOrCreateDataRegistry — Failed to create UOptionsDataRegistry."));

	return CreatedOwningDataRegistry;
}

void UWidget_OptionsScreen::OnResetBoundActionTriggered()
{
	if (ResettableDataArray.IsEmpty())
	{
		return;
	}

	// Retrieve the active tab button's display text for the confirmation message.
	UCommonButtonBase* SelectedTabButton = TabListWidget_OptionsTabs->GetTabButtonBaseByID(
		TabListWidget_OptionsTabs->GetActiveTab());

	const FString SelectedTabButtonName = CastChecked<UFrontendUICommonButtonBase>(SelectedTabButton)->
	                                      GetButtonDisplayText().ToString();

	// Show a confirmation dialog before resetting — destructive action requires explicit confirmation.
	UFrontendUISubsystem::Get(this)->PushConfirmScreenToModalStackAsync(EConfirmScreenType::YesNo,
	                                                                    FText::FromString(TEXT("Reset")),
	                                                                    FText::FromString(
		                                                                    TEXT(
			                                                                    "Are you sure you want to reset all the settings under the ")
		                                                                    + SelectedTabButtonName + TEXT(" tab?")),
	                                                                    [this](
                                                                    EConfirmScreenButtonType ClickedButtonType)
	                                                                    {
		                                                                    if (ClickedButtonType !=
			                                                                    EConfirmScreenButtonType::Confirmed)
		                                                                    {
			                                                                    return;
		                                                                    }

		                                                                    // Guard against OnListViewListDataModified modifying ResettableDataArray
		                                                                    // while we are iterating over it during the reset loop.
		                                                                    bIsResettingData = true;
		                                                                    bool bHasDataFailedToReset = false;

		                                                                    for (UListDataObject_Base* DataToReset :
			                                                                    ResettableDataArray)
		                                                                    {
			                                                                    if (!DataToReset)
			                                                                    {
				                                                                    continue;
			                                                                    }

			                                                                    if (!DataToReset->
				                                                                    TryResetBackToDefaultValue())
			                                                                    {
				                                                                    bHasDataFailedToReset = true;
			                                                                    }
		                                                                    }

		                                                                    // Only clear the resettable array and remove the Reset binding
		                                                                    // if all data objects were successfully reset.
		                                                                    if (!bHasDataFailedToReset)
		                                                                    {
			                                                                    ResettableDataArray.Empty();
			                                                                    RemoveActionBinding(ResetActionHandle);
		                                                                    }

		                                                                    bIsResettingData = false;
	                                                                    });
}

void UWidget_OptionsScreen::OnBackBoundActionTriggered()
{
	DeactivateWidget();
}

void UWidget_OptionsScreen::OnOptionsTabSelected(FName TabId)
{
	// Clear the details view when switching tabs to avoid showing stale info.
	DetailsView_ListEntryInfo->ClearDetailsViewInfo();

	// Populate the list view with the selected tab's data objects.
	TArray<UListDataObject_Base*> FoundListSourceItems = GetOrCreateDataRegistry()->
			GetListSourceItemsBySelectedTabID(TabId);

	CommonListView_OptionsList->SetListItems(FoundListSourceItems);
	CommonListView_OptionsList->RequestRefresh();

	// Navigate to and select the first item in the new tab.
	if (CommonListView_OptionsList->GetNumItems() != 0)
	{
		CommonListView_OptionsList->NavigateToIndex(0);
		CommonListView_OptionsList->SetSelectedIndex(0);
	}

	// Rebuild the resettable data array for the new tab.
	ResettableDataArray.Empty();

	for (UListDataObject_Base* FoundListSourceItem : FoundListSourceItems)
	{
		if (!FoundListSourceItem)
		{
			continue;
		}

		// Bind to OnListDataModified to track value changes for the Reset system.
		// IsBoundToObject guard prevents duplicate bindings on tab re-selection.
		if (!FoundListSourceItem->OnListDataModified.IsBoundToObject(this))
		{
			FoundListSourceItem->OnListDataModified.AddUObject(this,
			                                                   &ThisClass::OnListViewListDataModified);
		}

		if (FoundListSourceItem->CanResetBackToDefaultValue())
		{
			ResettableDataArray.AddUnique(FoundListSourceItem);
		}
	}

	// Show or hide the Reset action binding based on whether any data is resettable.
	if (ResettableDataArray.IsEmpty())
	{
		RemoveActionBinding(ResetActionHandle);
	}
	else
	{
		if (!GetActionBindings().Contains(ResetActionHandle))
		{
			AddActionBinding(ResetActionHandle);
		}
	}
}

void UWidget_OptionsScreen::OnListViewItemHovered(UObject* InHoveredItem,
                                                  bool bWasHovered) const
{
	if (!InHoveredItem)
	{
		return;
	}

	UWidget_ListEntry_Base* HoveredEntryWidget = CommonListView_OptionsList->GetEntryWidgetFromItem<
		UWidget_ListEntry_Base>(InHoveredItem);
	check(HoveredEntryWidget);
	HoveredEntryWidget->NativeOnListEntryWidgetHovered(bWasHovered);

	if (bWasHovered)
	{
		// Update the details view with the hovered item's information.
		DetailsView_ListEntryInfo->UpdateDetailsViewInfo(CastChecked<UListDataObject_Base>(InHoveredItem),
		                                                 TryGetEntryWidgetClassName(InHoveredItem));
	}
	else
	{
		// When unhovered, restore details view to show the currently selected item.
		// This prevents the details view from going blank when the cursor moves off an entry.
		if (UListDataObject_Base* SelectedItem = CommonListView_OptionsList->GetSelectedItem<UListDataObject_Base>())
		{
			DetailsView_ListEntryInfo->UpdateDetailsViewInfo(SelectedItem,
			                                                 TryGetEntryWidgetClassName(SelectedItem));
		}
	}
}

void UWidget_OptionsScreen::OnListViewItemSelected(UObject* InSelectedItem)
{
	if (!InSelectedItem)
	{
		return;
	}

	DetailsView_ListEntryInfo->UpdateDetailsViewInfo(CastChecked<UListDataObject_Base>(InSelectedItem),
	                                                 TryGetEntryWidgetClassName(InSelectedItem));
}

FString UWidget_OptionsScreen::TryGetEntryWidgetClassName(UObject* InOwningListItem) const
{
	if (UUserWidget* FoundEntryWidget = CommonListView_OptionsList->GetEntryWidgetFromItem(InOwningListItem))
	{
		return FoundEntryWidget->GetClass()->GetName();
	}

	return TEXT("Entry Widget Not Valid");
}

void UWidget_OptionsScreen::OnListViewListDataModified(UListDataObject_Base* ModifiedData,
                                                       EOptionsListDataModifyReason ModifyReason)
{
	// Skip updates during batch reset to prevent concurrent modification of ResettableDataArray.
	if (!ModifiedData || bIsResettingData)
	{
		return;
	}

	if (ModifiedData->CanResetBackToDefaultValue())
	{
		// Track this data object as resettable and ensure the Reset binding is active.
		ResettableDataArray.AddUnique(ModifiedData);

		if (!GetActionBindings().Contains(ResetActionHandle))
		{
			AddActionBinding(ResetActionHandle);
		}
	}
	else
	{
		// Value is back at default — remove from resettable array.
		ResettableDataArray.Remove(ModifiedData);
	}

	// Remove the Reset binding when no more resettable data exists.
	if (ResettableDataArray.IsEmpty())
	{
		RemoveActionBinding(ResetActionHandle);
	}
}
