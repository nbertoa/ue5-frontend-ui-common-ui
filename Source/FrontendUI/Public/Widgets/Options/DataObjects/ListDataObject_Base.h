#pragma once

#include "CoreMinimal.h"
#include "FrontendUITypes/FrontendUIEnumTypes.h"
#include "FrontendUITypes/FrontendUIStructTypes.h"
#include "ListDataObject_Base.generated.h"

/**
 * @brief Convenience macro for generating typed getter and setter for list data properties.
 *
 * Generates two inline functions:
 * - Get##PropertyName(): returns the property value by copy.
 * - Set##PropertyName(): sets the property value.
 *
 * Used throughout the ListDataObject hierarchy to reduce boilerplate
 * for simple value properties that do not require change notifications.
 * For properties that should trigger UI updates on change, use
 * NotifyListDataModified instead of a raw setter.
 *
 * @param DataType     The type of the property (e.g., FName, FText, bool).
 * @param PropertyName The name of the property (must match the private member name).
 */
#define LIST_DATA_ACCESSOR(DataType, PropertyName) \
FORCEINLINE DataType Get##PropertyName() const { return PropertyName; } \
void Set##PropertyName(DataType In##PropertyName) { PropertyName = In##PropertyName; }

/**
 * @class UListDataObject_Base
 * @brief Abstract base class for all data objects displayed in the options list view.
 *
 * Represents a single row of data in the options screen list. Each data object
 * carries display metadata (name, description, image) and behavioral state
 * (editability, edit conditions, dependency relationships).
 *
 * @section Architecture Data-Driven Options System
 * The options system uses a Model-View separation:
 * - Data objects (UListDataObject_Base and subclasses) own the data and business logic.
 * - Entry widgets (UWidget_ListEntry_Base and subclasses) own the visual representation.
 * - UFrontendUICommonListView connects them via UDataAsset_DataListEntryMapping.
 *
 * Data objects notify their widgets via OnListDataModified multicast delegate
 * whenever their value changes, keeping the UI in sync without polling.
 *
 * @section Edit Conditions
 * Edit conditions (FOptionsDataEditConditionDescriptor) define rules that determine
 * whether a setting is currently editable. Multiple conditions can be stacked.
 * When a condition is not met, the setting displays a rich-text disabled reason
 * and optionally forces its value to a specific string.
 *
 * @section Dependencies
 * Data objects can declare dependencies on other data objects via AddEditDependencyData.
 * When the dependency changes, OnEditDependencyDataModified is called, allowing
 * the dependent object to re-evaluate its edit conditions or update its value.
 *
 * @see UListDataObject_Value for data objects with getter/setter binding.
 * @see UListDataObject_Collection for tab category grouping.
 * @see UOptionsDataRegistry for how data objects are created and configured.
 */
UCLASS()
class FRONTENDUI_API UListDataObject_Base : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief Multicast delegate fired when this data object's value or state changes.
	 *
	 * Bound by UWidget_ListEntry_Base to update the visual representation.
	 * Also bound by UWidget_OptionsScreen to track resettable data.
	 *
	 * @param ModifiedData   The data object that changed (may be this or a child).
	 * @param ModifyReason   Why the change occurred (direct, dependency, or reset).
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnListDataModifiedDelegate,
	                                     UListDataObject_Base*,
	                                     EOptionsListDataModifyReason)
	FOnListDataModifiedDelegate OnListDataModified;

	/**
	 * @brief Delegate fired when a dependency data object changes.
	 *
	 * Bound by UWidget_ListEntry_Base to re-evaluate editability when
	 * a setting this one depends on changes (e.g., fullscreen mode affects resolution).
	 */
	FOnListDataModifiedDelegate OnDependencyDataModified;

	// -------------------------------------------------------------------------
	// ACCESSORS (generated via LIST_DATA_ACCESSOR macro)
	// -------------------------------------------------------------------------
	LIST_DATA_ACCESSOR(FName,
	                   DataID)
	LIST_DATA_ACCESSOR(FText,
	                   DataDisplayName)
	LIST_DATA_ACCESSOR(FText,
	                   DescriptionRichText)
	LIST_DATA_ACCESSOR(FText,
	                   DisabledRichText)
	LIST_DATA_ACCESSOR(TSoftObjectPtr<UTexture2D>,
	                   SoftDescriptionImage)
	LIST_DATA_ACCESSOR(UListDataObject_Base*,
	                   ParentData)

	/**
	 * @brief Initializes this data object by calling OnDataObjectInitialized.
	 * Called by UListDataObject_Collection::AddChildListData after parenting.
	 * Subclasses implement OnDataObjectInitialized to perform type-specific setup
	 * (e.g., populating option arrays, reading the current value from settings).
	 */
	void InitDataObject();

	/**
	 * @brief Returns all child data objects owned by this object.
	 * Empty in the base class. UListDataObject_Collection overrides this
	 * to return its child array.
	 */
	virtual TArray<UListDataObject_Base*> GetAllChildListData() const { return TArray<UListDataObject_Base*>(); }

	/** @brief Returns true if this object has any child data objects. */
	virtual bool HasAnyChildListData() const { return false; }

	/**
	 * @brief Controls whether settings changes are applied immediately or on deactivation.
	 * When true, the setter is called as soon as the user changes the value.
	 * When false, changes are batched and applied when the options screen is closed.
	 *
	 * @param bShouldApplyRightAway True to apply changes immediately.
	 */
	void SetShouldApplySettingsImmediately(bool bShouldApplyRightAway)
	{
		bShouldApplyChangeImmediately = bShouldApplyRightAway;
	}

	// -------------------------------------------------------------------------
	// RESET INTERFACE
	// -------------------------------------------------------------------------
	/** @brief Returns true if this data object has a defined default value to reset to. */
	virtual bool HasDefaultValue() const { return false; }

	/** @brief Returns true if the current value differs from the default and can be reset. */
	virtual bool CanResetBackToDefaultValue() const { return false; }

	/**
	 * @brief Attempts to reset this data object's value to its default.
	 * @return True if the reset was successful, false if it was not possible.
	 */
	virtual bool TryResetBackToDefaultValue() { return false; }

	// -------------------------------------------------------------------------
	// EDIT CONDITIONS
	// -------------------------------------------------------------------------

	/**
	 * @brief Adds an edit condition that determines whether this setting is editable.
	 * Multiple conditions can be added — all must be met for the setting to be editable.
	 *
	 * @param InEditCondition The condition descriptor to add.
	 */
	void AddEditCondition(const FOptionsDataEditConditionDescriptor& InEditCondition);

	/**
	 * @brief Registers a dependency on another data object.
	 * When InDependencyData changes, OnEditDependencyDataModified is called
	 * on this object, allowing it to re-evaluate its edit conditions.
	 *
	 * @param InDependencyData The data object this one depends on.
	 */
	void AddEditDependencyData(UListDataObject_Base* InDependencyData);

	/**
	 * @brief Evaluates all edit conditions and returns whether this setting is currently editable.
	 * Also handles forced value application when a condition is not met.
	 *
	 * @return True if all conditions are met and the setting is editable.
	 */
	bool IsDataCurrentlyEditable();

protected:
	/**
	 * @brief Called by InitDataObject to perform type-specific initialization.
	 * Empty in the base class. Subclasses override this to populate option arrays,
	 * read the current value from settings, and perform any other setup.
	 */
	virtual void OnDataObjectInitialized();

	/**
	 * @brief Broadcasts OnListDataModified to notify all listeners of a change.
	 *
	 * @param ModifiedData  The data object that changed.
	 * @param ModifyReason  Why the change occurred. Defaults to DirectlyModified.
	 */
	virtual void NotifyListDataModified(UListDataObject_Base* ModifiedData,
	                                    EOptionsListDataModifyReason ModifyReason =
			                                    EOptionsListDataModifyReason::DirectlyModified);

	/**
	 * @brief Returns true if this data object can accept the forced string value.
	 * Override in subclasses to validate the forced value before applying it.
	 *
	 * @param InForcedValue The value to check.
	 */
	virtual bool CanSetToForcedStringValue(const FString& InForcedValue) const { return false; }

	/**
	 * @brief Applies the forced string value to this data object.
	 * Called by IsDataCurrentlyEditable when a condition forces a specific value.
	 * Override in subclasses to implement the actual value assignment.
	 *
	 * @param InForcedValue The value to apply.
	 */
	virtual void OnSetToForcedStringValue(const FString& InForcedValue)
	{
	}

	/**
	 * @brief Called when a dependency data object's value changes.
	 * Override in subclasses to handle custom re-evaluation logic.
	 * Super call is expected to propagate the OnDependencyDataModified broadcast.
	 *
	 * @param ModifiedDependencyData The dependency that changed.
	 * @param ModifyReason           Why the dependency changed.
	 */
	virtual void OnEditDependencyDataModified(UListDataObject_Base* ModifiedDependencyData,
	                                          EOptionsListDataModifyReason ModifyReason);

private:
	/** @brief Unique identifier for this data object. Used as tab ID in the tab list. */
	FName DataID;

	/** @brief Localized display name shown in the list entry widget. */
	FText DataDisplayName;

	/** @brief Rich-text description shown in the options details view when this entry is selected. */
	FText DescriptionRichText;

	/** @brief Rich-text explanation shown when this entry is disabled by an edit condition. */
	FText DisabledRichText;

	/** @brief Optional soft reference to a texture displayed in the options details view. */
	TSoftObjectPtr<UTexture2D> SoftDescriptionImage;

	/** @brief Parent collection data object. Set by UListDataObject_Collection::AddChildListData. */
	UPROPERTY(Transient)
	UListDataObject_Base* ParentData;

	/**
	 * @brief If true, value changes are applied to GameUserSettings immediately on modification.
	 * If false, changes are batched and applied when NativeOnDeactivated fires on the options screen.
	 */
	bool bShouldApplyChangeImmediately = false;

	/**
	 * @brief Array of edit condition descriptors evaluated by IsDataCurrentlyEditable.
	 * All conditions must be met for the setting to be editable.
	 */
	UPROPERTY(Transient)
	TArray<FOptionsDataEditConditionDescriptor> EditConditionDescArray;
};
