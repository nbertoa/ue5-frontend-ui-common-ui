#pragma once

#include "CoreMinimal.h"
#include "Widgets/Options/DataObjects/ListDataObject_Base.h"
#include "ListDataObject_Collection.generated.h"

/**
 * @class UListDataObject_Collection
 * @brief Data object representing a tab category that groups child settings entries.
 *
 * Acts as a container node in the options data hierarchy. Each collection
 * corresponds to one tab in the options screen tab list (e.g., Gameplay, Audio,
 * Video, Controls). Its children are the individual settings entries displayed
 * in the list view when the corresponding tab is selected.
 *
 * @section Architecture Tree Structure
 * The options data is organized as a shallow two-level tree:
 *
 * @code
 * UListDataObject_Collection (Gameplay Tab)
 *   ├── UListDataObject_StringEnum  (Difficulty)
 *   └── ...
 * UListDataObject_Collection (Audio Tab)
 *   ├── UListDataObject_Scalar      (Overall Volume)
 *   ├── UListDataObject_Scalar      (Music Volume)
 *   ├── UListDataObject_StringBool  (Allow Background Audio)
 *   └── ...
 * @endcode
 *
 * Collections are registered as tabs in UFrontendUITabListWidgetBase.
 * Their children are set as the list source in UFrontendUICommonListView
 * when the corresponding tab is selected.
 *
 * Collections themselves are not selectable in the list view —
 * UFrontendUICommonListView::OnIsSelectableOrNavigableInternal blocks their selection.
 *
 * @see UOptionsDataRegistry for how collections and their children are created.
 * @see UWidget_OptionsScreen for how collections drive tab and list view updates.
 */
UCLASS()
class FRONTENDUI_API UListDataObject_Collection : public UListDataObject_Base
{
	GENERATED_BODY()

public:
	/**
	 * @brief Adds a child data object to this collection.
	 *
	 * Calls InitDataObject on the child (triggering its type-specific setup),
	 * sets this collection as the child's parent, and appends it to the child array.
	 * Order of addition determines display order in the options list view.
	 *
	 * @param InChildListData The data object to add as a child of this collection.
	 */
	void AddChildListData(UListDataObject_Base* InChildListData);

	//~ Begin UListDataObject_Base Interface
	/**
	 * @brief Returns all child data objects in this collection.
	 * Used by UOptionsDataRegistry::GetListSourceItemsBySelectedTabID
	 * to populate the list view when this collection's tab is selected.
	 */
	virtual TArray<UListDataObject_Base*> GetAllChildListData() const override;

	/** @brief Returns true if this collection has any child data objects. */
	virtual bool HasAnyChildListData() const override;
	//~ End UListDataObject_Base Interface

private:
	/**
	 * @brief Ordered array of child data objects belonging to this collection.
	 * Transient: rebuilt each session by UOptionsDataRegistry during initialization.
	 * Order determines display order in the options list view.
	 */
	UPROPERTY(Transient)
	TArray<UListDataObject_Base*> ChildListDataArray;
};
