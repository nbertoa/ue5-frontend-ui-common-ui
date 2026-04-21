#include "Widgets/Options/DataAsset_DataListEntryMapping.h"

#include "Widgets/Options/DataObjects/ListDataObject_Base.h"

TSubclassOf<UWidget_ListEntry_Base> UDataAsset_DataListEntryMapping::FindEntryWidgetClassByDataObject(
	const UListDataObject_Base* InDataObject) const
{
	check(InDataObject);

	// Walk the class hierarchy from most-derived to base class.
	// This ensures the most specific mapping is always preferred,
	// while allowing subclasses to fall back to their parent's mapping
	// if no explicit entry exists for their type.
	//
	// Example hierarchy walk for UListDataObject_StringBool:
	//   UListDataObject_StringBool → UListDataObject_String → UListDataObject_Value → UListDataObject_Base
	// The first class found in DataObjectListEntryMap wins.
	for (UClass* DataObjectClass = InDataObject->GetClass(); DataObjectClass; DataObjectClass = DataObjectClass->
	     GetSuperClass())
	{
		// Convert to TSubclassOf<UListDataObject_Base> for map key compatibility.
		if (TSubclassOf<UListDataObject_Base> ConvertedDataObjectClass = TSubclassOf<UListDataObject_Base>(
			DataObjectClass))
		{
			if (DataObjectListEntryMap.Contains(ConvertedDataObjectClass))
			{
				return DataObjectListEntryMap.FindRef(ConvertedDataObjectClass);
			}
		}
	}

	// No mapping found in the entire class hierarchy.
	// UFrontendUICommonListView falls back to the base class entry generation in this case.
	return TSubclassOf<UWidget_ListEntry_Base>();
}
