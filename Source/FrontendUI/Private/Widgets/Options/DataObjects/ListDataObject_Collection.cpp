#include "Widgets/Options/DataObjects/ListDataObject_Collection.h"

void UListDataObject_Collection::AddChildListData(UListDataObject_Base* InChildListData)
{
	// Initialize the child before parenting — OnDataObjectInitialized may read
	// the current value from GameUserSettings, which should happen before the
	// parent reference is set to avoid any order-dependent initialization issues.
	InChildListData->InitDataObject();

	// Set the parent reference so children can navigate up the hierarchy if needed.
	InChildListData->SetParentData(this);

	ChildListDataArray.Add(InChildListData);
}

TArray<UListDataObject_Base*> UListDataObject_Collection::GetAllChildListData() const
{
	return ChildListDataArray;
}

bool UListDataObject_Collection::HasAnyChildListData() const
{
	return !ChildListDataArray.IsEmpty();
}
