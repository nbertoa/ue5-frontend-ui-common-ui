#pragma once

#include "CoreMinimal.h"
#include "CommonTabListWidgetBase.h"
#include "FrontendUITabListWidgetBase.generated.h"

/**
 * @class UFrontendUITabListWidgetBase
 * @brief Project-specific tab list widget extending UCommonTabListWidgetBase.
 *
 * Wraps UCommonTabListWidgetBase to provide a simplified tab registration API
 * that combines tab registration and button text assignment in a single call.
 *
 * @section Architecture Common UI Tab System
 * UCommonTabListWidgetBase manages a set of tab buttons, each identified by FName.
 * When a tab is selected, it broadcasts OnTabSelected with the tab's ID, allowing
 * the owning screen (UWidget_OptionsScreen) to update the list view accordingly.
 *
 * Standard RegisterTab requires separate steps to register and configure each button.
 * RequestRegisterTab consolidates this into one call, reducing boilerplate in
 * UWidget_OptionsScreen::NativeOnActivated where tabs are registered dynamically
 * from UOptionsDataRegistry's collection array.
 *
 * @section Tab Button Class
 * TabButtonEntryWidgetClass defines which UFrontendUICommonButtonBase subclass
 * is instantiated for each tab. Must be assigned in the Blueprint subclass —
 * enforced by ValidateCompiledDefaults.
 *
 * @section DisableNaiveTick
 * meta = (DisableNaiveTick) prevents UMG from enabling Tick unless explicitly
 * requested. Tab selection is entirely event-driven via OnTabSelected.
 *
 * @see UWidget_OptionsScreen for how tabs are registered and selection is handled.
 * @see UOptionsDataRegistry for the data that drives tab creation.
 */
UCLASS(Abstract,
	BlueprintType,
	meta = (DisableNaiveTick))
class FRONTENDUI_API UFrontendUITabListWidgetBase : public UCommonTabListWidgetBase
{
	GENERATED_BODY()

public:
	/**
	 * @brief Registers a tab and sets its display text in a single call.
	 *
	 * Calls RegisterTab with the configured TabButtonEntryWidgetClass,
	 * then retrieves the created button and sets its display text via SetButtonText.
	 * This consolidates what would otherwise be two separate calls at every registration site.
	 *
	 * @param InTabID          Unique identifier for this tab (used by OnTabSelected).
	 * @param InTabDisplayName Localized text displayed on the tab button.
	 */
	void RequestRegisterTab(const FName& InTabID,
	                        const FText& InTabDisplayName);

private:
	//~ Begin UWidget Interface
#if WITH_EDITOR
	/**
	 * @brief Validates that TabButtonEntryWidgetClass is assigned before Blueprint compilation.
	 * Emits a compile error if missing, preventing silent runtime failures
	 * where tabs would be registered without a valid button widget class.
	 *
	 * @param CompileLog The compiler log to emit errors/warnings to.
	 */
	virtual void ValidateCompiledDefaults(class IWidgetCompilerLog& CompileLog) const override;
#endif
	//~ End UWidget Interface

	/**
	 * @brief Number of tabs to preview in the UMG editor at design time.
	 * Has no effect at runtime — tabs are registered dynamically via RequestRegisterTab.
	 * Clamped between 1 and 10 to keep the editor preview manageable.
	 */
	UPROPERTY(EditAnywhere,
		BlueprintReadOnly,
		Category = "FrontendUI Tab List Settings",
		meta = (AllowPrivateAccess = "true", ClampMin = "1", ClampMax = "10"))
	int32 DebugEditorPreviewTabCount = 3;

	/**
	 * @brief Widget class instantiated for each registered tab button.
	 * Must be a UFrontendUICommonButtonBase subclass configured in Blueprint.
	 * Enforced by ValidateCompiledDefaults — compile error if not assigned.
	 */
	UPROPERTY(EditAnywhere,
		BlueprintReadOnly,
		Category = "FrontendUI Tab List Settings",
		meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UFrontendUICommonButtonBase> TabButtonEntryWidgetClass;
};
