#include "Widgets/Options/DataObjects/ListDataObject_Base.h"

void UListDataObject_Base::InitDataObject()
{
	// Delegate to the virtual override so each subclass can perform
	// type-specific initialization (populating options, reading current values, etc.)
	OnDataObjectInitialized();
}

void UListDataObject_Base::OnDataObjectInitialized()
{
	// Base implementation is intentionally empty.
	// Subclasses override this to perform their specific initialization.
}

void UListDataObject_Base::NotifyListDataModified(UListDataObject_Base* ModifiedData,
                                                  EOptionsListDataModifyReason ModifyReason)
{
	// Broadcast to all listeners — entry widgets update their UI,
	// and UWidget_OptionsScreen tracks which data objects need reset buttons.
	OnListDataModified.Broadcast(ModifiedData,
	                             ModifyReason);
}

void UListDataObject_Base::AddEditCondition(const FOptionsDataEditConditionDescriptor& InEditCondition)
{
	EditConditionDescArray.Add(InEditCondition);
}

void UListDataObject_Base::AddEditDependencyData(UListDataObject_Base* InDependencyData)
{
	// Guard against duplicate bindings — if this object is already listening
	// to InDependencyData, do not bind again to prevent double callbacks.
	if (!InDependencyData->OnListDataModified.IsBoundToObject(this))
	{
		InDependencyData->OnListDataModified.AddUObject(this,
		                                                &ThisClass::OnEditDependencyDataModified);
	}
}

bool UListDataObject_Base::IsDataCurrentlyEditable()
{
	// If no conditions are registered, the setting is always editable.
	if (EditConditionDescArray.IsEmpty())
	{
		return true;
	}

	bool bIsEditable = true;
	FString CachedDisabledRichReason;

	for (const FOptionsDataEditConditionDescriptor& Condition : EditConditionDescArray)
	{
		// Skip invalid conditions (no function set) — treated as always met.
		if (!Condition.IsValid() || Condition.IsEditConditionMet())
		{
			continue;
		}

		// At least one condition is not met — mark as not editable.
		bIsEditable = false;

		// Accumulate disabled reasons from all unmet conditions.
		CachedDisabledRichReason.Append(Condition.GetDisabledRichReason());
		SetDisabledRichText(FText::FromString(CachedDisabledRichReason));

		// If this condition forces a specific value when unmet (e.g., lock resolution
		// to maximum when windowed mode is active), apply it now.
		if (Condition.HasForcedStringValue())
		{
			const FString ForcedStringValue = Condition.GetDisabledForcedStringValue();

			if (CanSetToForcedStringValue(ForcedStringValue))
			{
				OnSetToForcedStringValue(ForcedStringValue);
			}
		}
	}

	return bIsEditable;
}

void UListDataObject_Base::OnEditDependencyDataModified(UListDataObject_Base* ModifiedDependencyData,
                                                        EOptionsListDataModifyReason ModifyReason)
{
	// Propagate the dependency change notification to this object's own listeners
	// (e.g., entry widgets that need to re-evaluate their editable state).
	OnDependencyDataModified.Broadcast(ModifiedDependencyData,
	                                   ModifyReason);
}
