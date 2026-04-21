#include "Widgets/Widget_ActivatableBase.h"

#include "Controllers/FrontendUIPlayerController.h"

AFrontendUIPlayerController* UWidget_ActivatableBase::GetOwningFrontendUIPlayerController()
{
	// Populate the cache on first call or if the cached reference has been invalidated.
	// TWeakObjectPtr::IsValid() returns false if the object was garbage collected,
	// ensuring we never return a dangling pointer.
	if (!CachedOwningFrontendUIPC.IsValid())
	{
		CachedOwningFrontendUIPC = GetOwningPlayer<AFrontendUIPlayerController>();
	}

	// Return the cached pointer if still valid, nullptr otherwise.
	// Callers must null-check the result — this widget may be used in
	// contexts where the owning player is not an AFrontendUIPlayerController.
	return CachedOwningFrontendUIPC.IsValid()
		       ? CachedOwningFrontendUIPC.Get()
		       : nullptr;
}
