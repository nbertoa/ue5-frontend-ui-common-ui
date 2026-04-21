#pragma once

#include "CoreMinimal.h"
#include "Widgets/Widget_ActivatableBase.h"
#include "FrontendUITypes/FrontendUIEnumTypes.h"
#include "Widget_OptionsScreen.generated.h"

/**
 * @class UWidget_OptionsScreen
 * @brief The main options screen widget managing tabs, settings list, and details view.
 *
 * Coordinates three child widgets — a tab list, a settings list view, and a details panel —
 * and drives their state from UOptionsDataRegistry, which owns the data object hierarchy.
 *
 * @section Architecture Three-Panel Layout
 * The options screen consists of three coordinated panels:
 *
 * - UFrontendUITabListWidgetBase (TabListWidget_OptionsTabs):
 *   Displays one tab button per UListDataObject_Collection registered in UOptionsDataRegistry.
 *   Tab selection triggers OnOptionsTabSelected, which updates the list view.
 *
 * - UFrontendUICommonListView (CommonListView_OptionsList):
 *   Displays the child data objects of the selected tab collection as entry widgets.
 *   Hover and selection events update the details view.
 *
 * - UWidget_OptionsDetailsView (DetailsView_ListEntryInfo):
 *   Shows contextual information (name, description, image, disabled reason) for
 *   the selected or hovered list entry.
 *
 * @section Data Registry Lifecycle
 * UOptionsDataRegistry is created lazily on first access via GetOrCreateDataRegistry.
 * It is owned by this widget (UPROPERTY Transient) and destroyed with it.
 * Tabs are registered in NativeOnActivated (not NativeOnInitialized) because the registry
 * may not be ready at initialization time and to support re-activation after deactivation.
 *
 * @section Reset System
 * The Reset bound action is registered in NativeOnInitialized and dynamically added/removed
 * from the active action bindings based on whether any settings in the current tab
 * differ from their default values. ResettableDataArray tracks which data objects
 * have been modified and can be reset.
 *
 * @section Settings Persistence
 * Settings are applied via UFrontendUIGameUserSettings::ApplySettings in NativeOnDeactivated,
 * batching all changes made during the session into a single save/apply operation.
 *
 * @see UOptionsDataRegistry for how data objects are created and organized.
 * @see UWidget_ActivatableBase for the Common UI integration base class.
 */
UCLASS(Abstract,
	BlueprintType,
	meta = (DisableNaiveTick))
class FRONTENDUI_API UWidget_OptionsScreen : public UWidget_ActivatableBase
{
	GENERATED_BODY()

protected:
	//~ Begin UUserWidget Interface
	/**
	 * @brief Sets up action bindings (Reset, Back) and connects list view and tab list delegates.
	 * Called once when the widget is first constructed.
	 */
	virtual void NativeOnInitialized() override;
	//~ End UUserWidget Interface

private:
	//~ Begin UCommonActivatableWidget Interface
	/**
	 * @brief Registers tabs from the data registry and refreshes the list view.
	 * Called each time the widget is activated (pushed to the stack or returned to).
	 * Guards against duplicate tab registration via GetTabButtonBaseByID checks.
	 */
	virtual void NativeOnActivated() override;

	/**
	 * @brief Applies all pending settings changes to GameUserSettings on deactivation.
	 * Called when the widget is deactivated (popped from the stack or covered by another widget).
	 */
	virtual void NativeOnDeactivated() override;

	/**
	 * @brief Returns the selected list entry widget as the desired focus target.
	 * Falls back to the base class implementation if no item is selected.
	 */
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	//~ End UCommonActivatableWidget Interface

	/**
	 * @brief Returns the data registry, creating it if it does not yet exist.
	 * Lazy creation ensures the registry is only built when the options screen is first opened.
	 *
	 * @return The active UOptionsDataRegistry for this screen instance.
	 */
	class UOptionsDataRegistry* GetOrCreateDataRegistry();

	/** @brief Called when the Reset bound action is triggered. Shows a confirmation dialog. */
	void OnResetBoundActionTriggered();

	/** @brief Called when the Back bound action is triggered. Deactivates the widget. */
	void OnBackBoundActionTriggered();

	/**
	 * @brief Called when a tab is selected in the tab list.
	 * Updates the list view with the selected tab's data objects and
	 * refreshes the resettable data array and Reset action binding.
	 *
	 * @param TabId The FName identifier of the selected tab.
	 */
	UFUNCTION()
	void OnOptionsTabSelected(FName TabId);

	/**
	 * @brief Called when a list entry is hovered or unhovered.
	 * Forwards hover state to the entry widget and updates the details view.
	 *
	 * @param InHoveredItem The hovered list item object.
	 * @param bWasHovered   True if the item is now hovered, false if unhovered.
	 */
	void OnListViewItemHovered(UObject* InHoveredItem,
	                           bool bWasHovered) const;

	/**
	 * @brief Called when a list entry is selected.
	 * Updates the details view with the selected item's information.
	 *
	 * @param InSelectedItem The selected list item object.
	 */
	void OnListViewItemSelected(UObject* InSelectedItem);

	/**
	 * @brief Attempts to retrieve the entry widget class name for the given list item.
	 * Used to populate the dynamic debug details in UWidget_OptionsDetailsView.
	 *
	 * @param InOwningListItem The list item to look up.
	 * @return The entry widget's class name, or "Entry Widget Not Valid" if not found.
	 */
	FString TryGetEntryWidgetClassName(UObject* InOwningListItem) const;

	/**
	 * @brief Called when any list data object in the current tab is modified.
	 * Updates ResettableDataArray and toggles the Reset action binding accordingly.
	 *
	 * @param ModifiedData   The data object that changed.
	 * @param ModifyReason   Why the change occurred.
	 */
	void OnListViewListDataModified(class UListDataObject_Base* ModifiedData,
	                                EOptionsListDataModifyReason ModifyReason);

	// -------------------------------------------------------------------------
	// BOUND WIDGETS
	// -------------------------------------------------------------------------

	/** @brief Tab list widget. One tab button per UListDataObject_Collection in the registry. Must exist in Blueprint. */
	UPROPERTY(meta = (BindWidget))
	class UFrontendUITabListWidgetBase* TabListWidget_OptionsTabs;

	/** @brief Settings list view. Displays the active tab's child data objects. Must exist in Blueprint. */
	UPROPERTY(meta = (BindWidget))
	class UFrontendUICommonListView* CommonListView_OptionsList;

	/** @brief Details panel. Shows info for the selected or hovered list entry. Must exist in Blueprint. */
	UPROPERTY(meta = (BindWidget))
	class UWidget_OptionsDetailsView* DetailsView_ListEntryInfo;

	// -------------------------------------------------------------------------
	// STATE
	// -------------------------------------------------------------------------

	/**
	 * @brief The data registry owning all options data objects.
	 * Created lazily on first access via GetOrCreateDataRegistry.
	 * Transient: rebuilt each session when the options screen is first opened.
	 */
	UPROPERTY(Transient)
	UOptionsDataRegistry* CreatedOwningDataRegistry;

	/**
	 * @brief Data table row handle for the Reset input action.
	 * Configured in Blueprint — references a row in the Common UI input action data table.
	 */
	UPROPERTY(EditDefaultsOnly,
		Category = "FrontendUI Options Screen",
		meta = (RowType = "/Script/CommonUI.CommonInputActionDataBase"))
	FDataTableRowHandle ResetAction;

	/**
	 * @brief Handle for the Reset action binding.
	 * Stored to allow dynamic add/remove of the binding based on resettable data availability.
	 */
	FUIActionBindingHandle ResetActionHandle;

	/**
	 * @brief Array of data objects in the current tab that have been modified and can be reset.
	 * Populated when a tab is selected and updated as the user modifies settings.
	 * Empty when all settings are at their default values — triggers Reset action removal.
	 */
	UPROPERTY(Transient)
	TArray<class UListDataObject_Base*> ResettableDataArray;

	/**
	 * @brief Guard flag set during batch reset operations.
	 * Prevents OnListViewListDataModified from modifying ResettableDataArray
	 * while TryResetBackToDefaultValue is being called on each item in the array.
	 */
	bool bIsResettingData = false;
};
