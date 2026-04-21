#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAsset_DataListEntryMapping.generated.h"

/**
 * @class UDataAsset_DataListEntryMapping
 * @brief Data asset that maps data object types to their corresponding list entry widget classes.
 *
 * Used by UFrontendUICommonListView::OnGenerateEntryWidgetInternal to resolve
 * the correct entry widget class for each UListDataObject_Base subclass at runtime.
 *
 * @section Architecture Polymorphic Widget Resolution
 * The options list displays different data types (scalars, strings, collections)
 * that each require a different widget layout. Rather than hardcoding type checks
 * in the list view, this data asset externalizes the mapping so designers can
 * configure new data-to-widget associations without modifying C++ code.
 *
 * FindEntryWidgetClassByDataObject walks the data object's class hierarchy,
 * so subclasses of mapped types are handled automatically. For example, if
 * UListDataObject_String is mapped, UListDataObject_StringBool will also match
 * unless it has its own explicit mapping with higher priority.
 *
 * @section Usage
 * Assign this asset in the UFrontendUICommonListView Blueprint Details panel.
 * Populate DataObjectListEntryMap with data object class → entry widget class pairs.
 *
 * @see UFrontendUICommonListView for where this asset is used.
 * @see UListDataObject_Base and subclasses for the data object types to map.
 * @see UWidget_ListEntry_Base and subclasses for the entry widget types to map to.
 */
UCLASS()
class FRONTENDUI_API UDataAsset_DataListEntryMapping : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * @brief Finds the best matching entry widget class for a given data object.
	 *
	 * Walks the data object's class hierarchy from most-derived to base class,
	 * returning the first match found in DataObjectListEntryMap.
	 * This allows subclasses to inherit their parent's mapping automatically
	 * while still supporting explicit overrides per subclass.
	 *
	 * @param InDataObject The data object to find a widget class for. Must not be null.
	 * @return The matching entry widget class, or an empty TSubclassOf if no match is found.
	 */
	TSubclassOf<class UWidget_ListEntry_Base> FindEntryWidgetClassByDataObject(
		const class UListDataObject_Base* InDataObject) const;

private:
	/**
	 * @brief Map from data object class to entry widget class.
	 *
	 * Key:   A UListDataObject_Base subclass (e.g., UListDataObject_Scalar).
	 * Value: The corresponding UWidget_ListEntry_Base subclass to instantiate.
	 *
	 * Configured in the editor by assigning this data asset and populating the map.
	 * Hierarchy walking in FindEntryWidgetClassByDataObject means you only need
	 * to map the most specific type you want to customize — base types are inherited.
	 */
	UPROPERTY(EditDefaultsOnly)
	TMap<TSubclassOf<UListDataObject_Base>, TSubclassOf<UWidget_ListEntry_Base>> DataObjectListEntryMap;
};
