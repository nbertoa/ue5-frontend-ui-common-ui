#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Widget_ActivatableBase.generated.h"

/**
 * @class UWidget_ActivatableBase
 * @brief Base class for all activatable widgets in the FrontendUI module.
 *
 * Extends UCommonActivatableWidget to provide project-specific helpers
 * and enforce consistent patterns across all screens and overlays.
 *
 * @section Architecture Common UI Integration
 * UCommonActivatableWidget is the foundation of Common UI's navigation model.
 * Each activatable widget represents a discrete UI "layer" that can be pushed
 * onto a UCommonActivatableWidgetContainerBase (widget stack) and activated/
 * deactivated independently. When activated, it becomes the focus scope for
 * input routing — keyboard, gamepad, and back-button actions are handled
 * by the topmost active widget in the stack.
 *
 * @section DisableNaiveTick
 * meta = (DisableNaiveTick) prevents UMG from enabling Tick on this widget
 * unless explicitly requested. Ticking widgets is expensive at scale —
 * event-driven updates via delegates are always preferred in this codebase.
 *
 * @section PlayerController Caching
 * GetOwningFrontendUIPlayerController caches the result after the first call
 * using a TWeakObjectPtr. This avoids repeated GetOwningPlayer<> casts
 * on every access while remaining safe if the controller is destroyed.
 */
UCLASS(Abstract,
	BlueprintType,
	meta = (DisableNaiveTick))
class FRONTENDUI_API UWidget_ActivatableBase : public UCommonActivatableWidget
{
	GENERATED_BODY()

protected:
	/**
	 * @brief Returns the owning PlayerController cast to AFrontendUIPlayerController.
	 *
	 * Result is cached after the first valid call to avoid repeated casts.
	 * Returns nullptr if the owning player is not an AFrontendUIPlayerController
	 * (e.g., if this widget is used outside the frontend game mode).
	 *
	 * @return Pointer to the owning AFrontendUIPlayerController, or nullptr.
	 */
	UFUNCTION(BlueprintPure)
	class AFrontendUIPlayerController* GetOwningFrontendUIPlayerController();

private:
	/**
	 * @brief Cached weak reference to the owning AFrontendUIPlayerController.
	 *
	 * TWeakObjectPtr prevents this widget from holding a strong reference
	 * to the controller, allowing normal garbage collection if the controller
	 * is destroyed (e.g., during level transitions or session end).
	 */
	TWeakObjectPtr<class AFrontendUIPlayerController> CachedOwningFrontendUIPC;
};
