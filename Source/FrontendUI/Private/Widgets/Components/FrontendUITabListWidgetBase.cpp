#include "Widgets/Components/FrontendUITabListWidgetBase.h"

#include "Editor/WidgetCompilerLog.h"
#include "Widgets/Components/FrontendUICommonButtonBase.h"

void UFrontendUITabListWidgetBase::RequestRegisterTab(const FName& InTabID,
                                                      const FText& InTabDisplayName)
{
	// Register the tab with the Common UI tab system using the configured button class.
	// The third parameter (content widget) is null — tabs here are navigation-only,
	// not content containers. Content is driven by the options list view instead.
	RegisterTab(InTabID,
	            TabButtonEntryWidgetClass,
	            nullptr);

	// Retrieve the created button and set its display text.
	// GetTabButtonBaseByID returns the button immediately after RegisterTab,
	// so the cast is safe as long as TabButtonEntryWidgetClass is correctly configured.
	if (UFrontendUICommonButtonBase* FoundButton = Cast<UFrontendUICommonButtonBase>(GetTabButtonBaseByID(InTabID)))
	{
		FoundButton->SetButtonText(InTabDisplayName);
	}
}

#if WITH_EDITOR
void UFrontendUITabListWidgetBase::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);

	// Enforce that the tab button class is assigned before the Blueprint is compiled.
	// Without it, RegisterTab will fail silently at runtime and no tabs will appear.
	if (!TabButtonEntryWidgetClass)
	{
		CompileLog.Error(FText::FromString(
			TEXT("UFrontendUITabListWidgetBase: TabButtonEntryWidgetClass is not assigned on ") + GetClass()->GetName()
			+ TEXT(". Assign a valid UFrontendUICommonButtonBase subclass in the Blueprint Details panel.")));
	}
}
#endif
