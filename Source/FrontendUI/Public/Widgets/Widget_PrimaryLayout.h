#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Widget_PrimaryLayout.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPrimaryLayout,
                            Log,
                            All);

/**
 * @class UWidget_PrimaryLayout
 * @brief Root UI widget that owns and manages all named widget stacks.
 *
 * The primary layout is the top-level widget added to the viewport once
 * per game session. It acts as a container registry — Blueprint subclasses
 * define the actual UCommonActivatableWidgetContainerBase instances in UMG,
 * then register them via RegisterWidgetStack during NativeOnInitialized.
 *
 * @section Architecture Tag-Based Stack Registry
 * Widget stacks are identified by Gameplay Tags (FrontendUI.WidgetStack.*)
 * rather than direct object references. This allows any C++ system to push
 * widgets onto a specific stack without holding a pointer to the layout widget
 * or knowing its internal structure.
 *
 * Stack lookup is O(1) via TMap<FGameplayTag, UCommonActivatableWidgetContainerBase*>.
 *
 * @section Stack Types
 * - Frontend:  Title screen and pre-game UI.
 * - GameHud:   In-game HUD elements.
 * - GameMenu:  Pause menus and in-game overlays.
 * - Modal:     Blocking dialogs (confirmation screens) that sit above all other stacks.
 *
 * @section Lifecycle
 * 1. Blueprint subclass adds this widget to the viewport.
 * 2. NativeOnInitialized fires — Blueprint calls RegisterWidgetStack for each stack.
 * 3. UFrontendUISubsystem::RegisterCreatedPrimaryLayoutWidget is called to give
 *    the subsystem a reference for future widget push requests.
 * 4. Widgets are pushed via UFrontendUISubsystem::PushSoftWidgetToStackAsync,
 *    which calls FindWidgetStackByTag to resolve the target stack.
 *
 * @see UFrontendUISubsystem for the widget push API.
 * @see FrontendUIGameplayTags for the stack tag constants.
 */
UCLASS(Abstract,
	BlueprintType,
	meta = (DisableNaiveTick))
class FRONTENDUI_API UWidget_PrimaryLayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * @brief Finds a registered widget stack by its Gameplay Tag.
	 *
	 * Called by UFrontendUISubsystem::PushSoftWidgetToStackAsync to resolve
	 * the target stack before pushing a widget onto it.
	 *
	 * @warning Triggers a checkf crash if the tag is not registered.
	 *          Ensure all stacks are registered via RegisterWidgetStack during initialization.
	 *
	 * @param InTag The stack tag to look up (e.g., FrontendUIGameplayTags::FrontendUI_WidgetStack_Modal).
	 * @return Pointer to the matching UCommonActivatableWidgetContainerBase.
	 */
	class UCommonActivatableWidgetContainerBase* FindWidgetStackByTag(const FGameplayTag& InTag) const;

protected:
	/**
	 * @brief Registers a widget stack under a Gameplay Tag.
	 *
	 * Called from Blueprint (NativeOnInitialized) to associate each
	 * UCommonActivatableWidgetContainerBase instance with its identifying tag.
	 * Duplicate registrations are rejected with an error log.
	 * Has no effect at design time (IsDesignTime() guard).
	 *
	 * @param InStackTag The tag to associate with this stack (FrontendUI.WidgetStack category).
	 * @param InStack    The widget stack container to register.
	 */
	UFUNCTION(BlueprintCallable)
	void RegisterWidgetStack(UPARAM(meta = (Categories = "FrontendUI.WidgetStack")) FGameplayTag InStackTag,
	                         UCommonActivatableWidgetContainerBase* InStack);

private:
	/**
	 * @brief Tag-to-stack registry populated during initialization.
	 *
	 * Transient: not serialized — rebuilt each session when Blueprint
	 * calls RegisterWidgetStack during NativeOnInitialized.
	 * TMap provides O(1) lookup by tag for widget push requests.
	 */
	UPROPERTY(Transient)
	TMap<FGameplayTag, class UCommonActivatableWidgetContainerBase*> WidgetStackByTag;
};
