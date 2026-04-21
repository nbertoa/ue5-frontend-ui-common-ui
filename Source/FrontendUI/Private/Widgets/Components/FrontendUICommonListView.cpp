#include "Widgets/Components/FrontendUICommonListView.h"

#include "Editor/WidgetCompilerLog.h"
#include "Widgets/Options/DataAsset_DataListEntryMapping.h"
#include "Widgets/Options/ListEntries/Widget_ListEntry_Base.h"
#include "Widgets/Options/DataObjects/ListDataObject_Base.h"
#include "Widgets/Options/DataObjects/ListDataObject_Collection.h"

UUserWidget& UFrontendUICommonListView::OnGenerateEntryWidgetInternal(
	UObject* Item,
	TSubclassOf<UUserWidget> DesiredEntryClass,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	// Check if the widget is being rendered in the Unreal Editor (Design Time)
	// In that case, just call the parent implementation and return its result.
	if (IsDesignTime())
	{
		return Super::OnGenerateEntryWidgetInternal(Item, DesiredEntryClass, OwnerTable);
	}

	// Retrieve the appropriate widget class to use for this data object.
	// The DataListEntryMapping maps data object types to their corresponding UI widget classes.
	TSubclassOf<UWidget_ListEntry_Base> FoundWidgetClass =
		DataListEntryMapping->FindEntryWidgetClassByDataObject(CastChecked<UListDataObject_Base>(Item));

	if (FoundWidgetClass)
	{
		// Generate and return a typed list entry widget using the class found above.
		// This ensures the correct type is used based on the data associated with this entry.
		return GenerateTypedEntry<UWidget_ListEntry_Base>(FoundWidgetClass, OwnerTable);
	}
	else
	{
		return Super::OnGenerateEntryWidgetInternal(Item, DesiredEntryClass, OwnerTable);
	}
}

bool UFrontendUICommonListView::OnIsSelectableOrNavigableInternal(UObject* FirstSelectedItem)
{
	return !FirstSelectedItem->IsA<UListDataObject_Collection>();
}

#if WITH_EDITOR	
void UFrontendUICommonListView::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);

	if (!DataListEntryMapping)
	{
		CompileLog.Error(FText::FromString(
		TEXT("The variable DataListEntryMapping has no valid data asset assigned ") +
		GetClass()->GetName() +
		TEXT(" needs a valid data asset to function properly")
		));
	}
}
#endif