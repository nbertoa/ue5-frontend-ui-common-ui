#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "FrontendUITypes/FrontendUIEnumTypes.h"
#include "Widget_ListEntry_Base.generated.h"

/**
 * @class UWidget_ListEntry_Base
 * @brief Abstract base class for all options list entry widgets.
 *
 * Implements IUserObjectListEntry to integrate with UCommonListView's
 * entry generation system. Each entry widget is associated with a
 * UListDataObject_Base subclass and displays its data visually.
 *
 * @section Architecture View Layer of the Options System
 * Entry widgets are the View in the Model-View separation of the options system:
 * - They receive data via NativeOnListItemObjectSet (called by the list view).
 * - They update their visuals via OnOwningListDataObjectModified (called by delegates).
 * - They never directly read from GameUserSettings — all data comes through the data object.
 *
 * @section Delegate Binding Lifecycle
 * When NativeOnListItemObjectSet fires:
 * 1. OnOwningListDataObjectSet is called — sets up initial visual state.
 * 2. OnListDataModified delegate is bound — future changes trigger OnOwningListDataObjectModified.
 * 3. OnDependencyDataModified delegate is bound — dependency changes trigger OnOwningDependencyDataObjectModified.
 *
 * When NativeOnEntryReleased fires (entry recycled by the list view):
 * 1. Hover state is cleared via NativeOnListEntryWidgetHovered(false).
 * Note: delegates are NOT unbound here — UListDataObject_Base guards against duplicate bindings.
 *
 * @section Gamepad Focus
 * NativeOnFocusReceived routes focus to the widget returned by BP_GetWidgetToFocusForGamepad
 * when a gamepad is active, allowing subclasses to direct focus to their interactive control
 * (e.g., the slider in UWidget_ListEntry_Scalar, the rotator in UWidget_ListEntry_String).
 *
 * @section DisableNaiveTick
 * meta = (DisableNaiveTick) prevents UMG from enabling Tick unless explicitly requested.
 * All updates are event-driven via data object delegates.
 *
 * @see UListDataObject_Base for the data model this widget displays.
 * @see UFrontendUICommonListView for how entry widgets are generated and recycled.
 */
UCLASS(Abstract,
	BlueprintType,
	meta = (DisableNaiveTick))
class FRONTENDUI_API UWidget_ListEntry_Base : public UCommonUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	/**
	 * @brief Blueprint event fired when this entry widget is hovered or unhovered.
	 *
	 * Implement in Blueprint to apply visual hover feedback (highlight, scale, etc.).
	 *
	 * @param bWasHovered              True if the entry is now hovered, false if unhovered.
	 * @param bIsEntryWidgetStillSelected True if this entry is still the selected item.
	 */
	UFUNCTION(BlueprintImplementableEvent,
		meta = (DisplayName = "On List Entry Widget Hovered"))
	void BP_OnListEntryWidgetHovered(bool bWasHovered,
	                                 bool bIsEntryWidgetStillSelected);

	/**
	 * @brief Called by UWidget_OptionsScreen when hover state changes.
	 * Resolves the selection state and forwards to BP_OnListEntryWidgetHovered.
	 *
	 * @param bWasHovered True if the entry is now hovered.
	 */
	void NativeOnListEntryWidgetHovered(bool bWasHovered);

protected:
	/**
	 * @brief Blueprint event that returns the widget to focus for gamepad navigation.
	 *
	 * Implement in Blueprint to return the interactive control within this entry
	 * (e.g., the slider widget for scalar entries, the rotator for string entries).
	 * Called by NativeOnFocusReceived when a gamepad is the active input device.
	 *
	 * @return The widget that should receive gamepad focus, or nullptr for default behavior.
	 */
	UFUNCTION(BlueprintImplementableEvent,
		meta = (DisplayName = "Get Widget To Focus For Gamepad"))
	UWidget* BP_GetWidgetToFocusForGamepad() const;

	//~ Begin IUserObjectListEntry Interface
	/**
	 * @brief Called by the list view when this entry is assigned a data object.
	 * Casts the list item to UListDataObject_Base and calls OnOwningListDataObjectSet.
	 */
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	/**
	 * @brief Called by the list view when this entry is recycled (removed from view).
	 * Clears hover state to prevent stale visual feedback on reuse.
	 */
	virtual void NativeOnEntryReleased() override;
	//~ End IUserObjectListEntry Interface

	//~ Begin UUserWidget Interface
	/**
	 * @brief Routes focus to the gamepad-designated widget when a gamepad is active.
	 * Falls back to default UMG focus behavior for non-gamepad input.
	 */
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry,
	                                     const FFocusEvent& InFocusEvent) override;
	//~ End UUserWidget Interface

	/**
	 * @brief Called when a data object is assigned to this entry widget.
	 *
	 * Sets the display name text, binds OnListDataModified and OnDependencyDataModified
	 * delegates, evaluates the initial editable state, and caches the data object.
	 * Subclasses override this to perform type-specific initialization (e.g., populating
	 * a slider with the scalar's current value and range).
	 * Super call is expected.
	 *
	 * @param InOwningListDataObject The data object assigned to this entry.
	 */
	virtual void OnOwningListDataObjectSet(class UListDataObject_Base* InOwningListDataObject);

	/**
	 * @brief Called when a dependency data object changes.
	 *
	 * Re-evaluates the editable state of this entry based on its data object's
	 * edit conditions. Subclasses may override for additional dependency handling.
	 * Super call is expected.
	 *
	 * @param OwningModifiedDependencyData The dependency that changed.
	 * @param ModifyReason                 Why the dependency changed.
	 */
	virtual void OnOwningDependencyDataObjectModified(UListDataObject_Base* OwningModifiedDependencyData,
	                                                  EOptionsListDataModifyReason ModifyReason);

	/**
	 * @brief Called when this entry's data object value changes.
	 *
	 * Empty in the base class. Subclasses override to update their specific
	 * interactive controls (e.g., update slider value, update rotator selection).
	 * Super call is NOT required — base class has no logic here.
	 *
	 * @param OwningModifiedData The data object that changed.
	 * @param ModifyReason       Why the change occurred.
	 */
	virtual void OnOwningListDataObjectModified(UListDataObject_Base* OwningModifiedData,
	                                            EOptionsListDataModifyReason ModifyReason);

	/**
	 * @brief Toggles the editable state of this entry widget's interactive controls.
	 *
	 * Base implementation disables CommonText_SettingDisplayName when not editable.
	 * Subclasses override to also disable their specific controls (slider, rotator, buttons).
	 * Super call is expected.
	 *
	 * @param bIsEditable True if the setting should be interactable.
	 */
	virtual void OnToggleEditableState(bool bIsEditable);

	/**
	 * @brief Programmatically selects this entry in the owning list view.
	 * Used by subclasses to ensure selection follows interactive control interactions
	 * (e.g., selecting the entry when the slider is dragged or a rotator button is clicked).
	 */
	void SelectThisEntryWidget() const;

private:
	/**
	 * @brief Optional bound text block for the setting's display name.
	 * BindWidgetOptional: entries without a display name label are valid.
	 * Disabled when the setting is not editable via OnToggleEditableState.
	 */
	UPROPERTY(BlueprintReadOnly,
		meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	class UCommonTextBlock* CommonText_SettingDisplayName;

	/**
	 * @brief Cached reference to the data object currently assigned to this entry.
	 * Set in OnOwningListDataObjectSet, used for editable state re-evaluation
	 * in OnOwningDependencyDataObjectModified.
	 */
	UPROPERTY(Transient)
	UListDataObject_Base* CachedOwningDataObject;
};
