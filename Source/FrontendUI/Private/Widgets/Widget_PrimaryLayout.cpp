#include "Widgets/Widget_PrimaryLayout.h"

DEFINE_LOG_CATEGORY(LogPrimaryLayout);

UCommonActivatableWidgetContainerBase* UWidget_PrimaryLayout::FindWidgetStackByTag(const FGameplayTag& InTag) const
{
	// Hard failure: requesting an unregistered stack is a programming error.
	// All stacks must be registered via RegisterWidgetStack during initialization.
	checkf(WidgetStackByTag.Contains(InTag),
	       TEXT("UWidget_PrimaryLayout::FindWidgetStackByTag — No stack registered for tag: %s. "
		       "Ensure RegisterWidgetStack is called for this tag during NativeOnInitialized."),
	       *InTag.ToString());

	return WidgetStackByTag.FindRef(InTag);
}

void UWidget_PrimaryLayout::RegisterWidgetStack(
	UPARAM(meta = (Categories = "FrontendUI.WidgetStack")) FGameplayTag InStackTag,
	UCommonActivatableWidgetContainerBase* InStack)
{
	check(InStack);

	// Skip registration at design time — widget stacks are runtime objects
	// and should not be registered in the editor preview context.
	if (IsDesignTime())
	{
		return;
	}

	if (!WidgetStackByTag.Contains(InStackTag))
	{
		WidgetStackByTag.Add(InStackTag,
		                     InStack);

		UE_LOG(LogPrimaryLayout,
		       Log,
		       TEXT("Widget stack registered: %s"),
		       *InStackTag.GetTagName().ToString());
	}
	else
	{
		// Duplicate registration indicates a Blueprint setup error —
		// RegisterWidgetStack is being called twice for the same tag.
		UE_LOG(LogPrimaryLayout,
		       Error,
		       TEXT("UWidget_PrimaryLayout::RegisterWidgetStack — Stack already registered for tag: %s. "
			       "Ensure RegisterWidgetStack is called only once per stack tag."),
		       *InStackTag.GetTagName().ToString());
	}
}
