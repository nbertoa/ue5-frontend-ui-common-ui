#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GameplayTagContainer.h"
#include "AsyncAction_PushSoftWidget.generated.h"

/**
 * @brief Delegate fired during the async widget push flow.
 *
 * @param PushedWidget The widget instance that was created or pushed.
 *        During OnWidgetCreatedBeforePush: the widget exists but is not yet visible.
 *        During AfterPush: the widget is active and visible in the stack.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPushSoftWidgetDelegate,
                                            UWidget_ActivatableBase*,
                                            PushedWidget);

/**
 * @class UAsyncAction_PushSoftWidget
 * @brief Async Blueprint node that loads and pushes a soft widget class onto a named stack.
 *
 * Wraps UFrontendUISubsystem::PushSoftWidgetToStackAsync in a UBlueprintAsyncActionBase,
 * exposing the two-phase push flow (OnWidgetCreatedBeforePush → AfterPush) as
 * separate Blueprint output execution pins.
 *
 * @section Why Two Output Pins?
 * - OnWidgetCreatedBeforePush: fires before the widget is visible. Use this pin
 *   to initialize the widget (e.g., pass data, set owning player) before it appears.
 * - AfterPush: fires after the widget is active in the stack. Use this pin
 *   to set focus or trigger entrance animations.
 *
 * @section Soft Class Loading
 * The widget class is a TSoftClassPtr — it is not loaded into memory until this
 * node activates. This keeps startup memory usage low and avoids loading widget
 * Blueprints that may never be shown in a given session.
 *
 * @see UFrontendUISubsystem::PushSoftWidgetToStackAsync
 * @see FrontendUIGameplayTags for valid widget stack tag constants.
 */
UCLASS()
class FRONTENDUI_API UAsyncAction_PushSoftWidget : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * @brief Factory function that creates and registers the async push node.
	 *
	 * BlueprintInternalUseOnly ensures this appears as an async node with output
	 * execution pins rather than a regular function in the Blueprint palette.
	 *
	 * @param InWorldContextObject        Any UObject with a valid world (e.g., the calling widget).
	 * @param InOwningPlayerController    The PlayerController to assign as the widget's owner.
	 * @param InSoftWidgetClass           Soft reference to the widget class to load and push.
	 * @param InWidgetStackTag            Tag identifying the target stack (must be FrontendUI.WidgetStack category).
	 * @param bInFocusOnNewlyPushedWidget If true, automatically sets focus to the widget's desired focus target after push.
	 * @return The initialized async action node, or nullptr if the world could not be resolved.
	 */
	UFUNCTION(BlueprintCallable,
		meta = (WorldContext = "InWorldContextObject", HidePin = "InWorldContextObject", BlueprintInternalUseOnly =
			"true", DisplayName = "Push Soft Widget To Widget Stack"))
	static UAsyncAction_PushSoftWidget* PushSoftWidget(const UObject* InWorldContextObject,
	                                                   APlayerController* InOwningPlayerController,
	                                                   TSoftClassPtr<UWidget_ActivatableBase> InSoftWidgetClass,
	                                                   UPARAM(meta = (Categories = "FrontendUI.WidgetStack"))
	                                                   FGameplayTag InWidgetStackTag,
	                                                   bool bInFocusOnNewlyPushedWidget = true);

	//~ Begin UBlueprintAsyncActionBase Interface
	/** @brief Activates the node and initiates the async widget push via UFrontendUISubsystem. */
	virtual void Activate() override;
	//~ End UBlueprintAsyncActionBase Interface

	/**
	 * @brief Fires when the widget has been created but not yet pushed to the stack.
	 * Use this pin to initialize the widget before it becomes visible.
	 */
	UPROPERTY(BlueprintAssignable)
	FOnPushSoftWidgetDelegate OnWidgetCreatedBeforePush;

	/**
	 * @brief Fires after the widget has been pushed and is active in the stack.
	 * Use this pin to set focus or trigger entrance animations.
	 */
	UPROPERTY(BlueprintAssignable)
	FOnPushSoftWidgetDelegate AfterPush;

private:
	/** @brief Cached world reference used to retrieve the subsystem during Activate(). */
	TWeakObjectPtr<UWorld> CachedOwningWorld;

	/** @brief Cached PlayerController assigned as the widget's owning player. */
	TWeakObjectPtr<APlayerController> CachedOwningPC;

	/** @brief Soft reference to the widget class to async-load and push. */
	TSoftClassPtr<UWidget_ActivatableBase> CachedSoftWidgetClass;

	/** @brief Tag identifying the target widget stack in UWidget_PrimaryLayout. */
	FGameplayTag CachedWidgetStackTag;

	/**
	 * @brief If true, focus is set to the pushed widget's desired focus target after AfterPush.
	 * Useful for gamepad/keyboard navigation to ensure the correct widget receives input.
	 */
	bool bCachedFocusOnNewlyPushedWidget = false;
};
