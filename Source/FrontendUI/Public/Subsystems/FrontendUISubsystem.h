#pragma once

#include "CoreMinimal.h"
#include "FrontendUITypes/FrontendUIEnumTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FrontendUISubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFrontendUISubsystem,
                            Log,
                            All);

/**
 * @brief Internal state enum for the two-phase async widget push callback.
 *
 * Used by PushSoftWidgetToStackAsync to distinguish between the moment
 * a widget is created (before being added to the stack) and the moment
 * it has been fully pushed and is visible.
 *
 * Not exposed to Blueprint — callers use the TFunction callback pattern instead.
 */
enum class EAsyncPushWidgetState : uint8
{
	/** Widget has been created and initialized but not yet added to the stack. Use this phase to configure the widget. */
	OnCreatedBeforePush,

	/** Widget has been pushed to the stack and is now active and visible. Use this phase for post-push logic. */
	AfterPush
};

/**
 * @brief Delegate broadcast when a button's description text should be shown or cleared.
 *
 * Fired by UFrontendUICommonButtonBase on hover/unhover.
 * Listeners (e.g., a description text block in the HUD) bind to this delegate
 * to display contextual help text without coupling the button to a specific UI layout.
 *
 * @param BroadcastingButton The button that triggered the update.
 * @param DescriptionText    The text to display. Empty FText signals the description should be cleared.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnButtonDescriptionTextUpdatedDelegate,
                                             class UFrontendUICommonButtonBase*,
                                             BroadcastingButton,
                                             FText,
                                             DescriptionText);

/**
 * @class UFrontendUISubsystem
 * @brief Central coordinator for the Frontend UI module.
 *
 * Manages the lifecycle of the primary layout widget and provides
 * the core API for pushing widgets onto named stacks asynchronously.
 *
 * @section Architecture GameInstance Subsystem
 * Lives on the GameInstance, giving it the same lifetime as the game session.
 * This makes it safely accessible from any world context without requiring
 * actor references or singleton patterns.
 *
 * Only one instance is created per game session. Derived classes are supported
 * (ShouldCreateSubsystem checks for derived classes and skips the base if one exists).
 * Not created on Dedicated Servers — frontend UI is client-only.
 *
 * @section Widget Push Flow
 * 1. Caller requests a widget push via PushSoftWidgetToStackAsync.
 * 2. The subsystem async-loads the widget class via AssetManager's StreamableManager.
 * 3. Once loaded, the widget is created inside the target stack (found via tag).
 * 4. A two-phase callback (OnCreatedBeforePush → AfterPush) allows the caller
 *    to configure the widget before it becomes visible and react after it is shown.
 *
 * @see UWidget_PrimaryLayout for the layout widget that owns the named stacks.
 * @see UFrontendUIFunctionLibrary for tag-based widget class resolution.
 */
UCLASS()
class FRONTENDUI_API UFrontendUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * @brief Type-safe accessor for this subsystem from any world context object.
	 *
	 * Retrieves the subsystem from the GameInstance associated with the given world.
	 * Asserts on failure — a null world context is a programming error in this context.
	 *
	 * @param WorldContextObject Any UObject with a valid world (Actor, Widget, Component).
	 * @return Pointer to the active UFrontendUISubsystem, or nullptr if GEngine is invalid.
	 */
	static UFrontendUISubsystem* Get(const UObject* WorldContextObject);

	//~ Begin USubsystem Interface
	/**
	 * @brief Prevents subsystem creation on Dedicated Servers and when a derived class exists.
	 *
	 * Frontend UI is client-only — no widgets should be created on server.
	 * If a project-specific derived class of UFrontendUISubsystem exists,
	 * the base class is skipped to avoid duplicate subsystem instances.
	 *
	 * @param Outer The GameInstance that would own this subsystem.
	 * @return True if this subsystem should be created for the given GameInstance.
	 */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End USubsystem Interface

	/**
	 * @brief Registers the primary layout widget created by the HUD or game mode.
	 *
	 * Must be called once after the primary layout widget is added to the viewport.
	 * The subsystem stores a reference to it for use in subsequent widget push requests.
	 *
	 * @param InCreatedWidget The newly created UWidget_PrimaryLayout instance.
	 */
	UFUNCTION(BlueprintCallable)
	void RegisterCreatedPrimaryLayoutWidget(UWidget_PrimaryLayout* InCreatedWidget);

	/**
	 * @brief Asynchronously loads and pushes a widget onto a named stack.
	 *
	 * Resolves the soft class reference via AssetManager's StreamableManager,
	 * then creates the widget inside the stack identified by InWidgetStackTag.
	 * The callback fires twice: once before the push (for configuration) and
	 * once after (for post-push logic like setting focus).
	 *
	 * @param InWidgetStackTag        Tag identifying the target stack in UWidget_PrimaryLayout.
	 * @param InSoftWidgetClass       Soft reference to the widget class to load and push.
	 * @param InAsyncPushStateCallback Two-phase callback: OnCreatedBeforePush and AfterPush.
	 */
	void PushSoftWidgetToStackAsync(const struct FGameplayTag& InWidgetStackTag,
	                                TSoftClassPtr<class UWidget_ActivatableBase> InSoftWidgetClass,
	                                TFunction<void(EAsyncPushWidgetState,
	                                               UWidget_ActivatableBase*)> InAsyncPushStateCallback) const;

	/**
	 * @brief Asynchronously pushes a typed confirmation screen onto the modal stack.
	 *
	 * Creates a UConfirmScreenInfoObject based on InScreenType, then async-loads
	 * and pushes the confirm screen widget onto the modal stack. The button clicked
	 * callback fires when the user interacts with any button on the screen.
	 *
	 * @param InScreenType           Layout of the confirmation screen (Ok, YesNo, OKCancel).
	 * @param InScreenTitle          Title text displayed at the top of the screen.
	 * @param InScreenMsg            Body message text displayed on the screen.
	 * @param InButtonClickedCallback Called with the button type when the user clicks a button.
	 */
	void PushConfirmScreenToModalStackAsync(EConfirmScreenType InScreenType,
	                                        const FText& InScreenTitle,
	                                        const FText& InScreenMsg,
	                                        TFunction<void(EConfirmScreenButtonType)> InButtonClickedCallback);

	/**
	 * @brief Broadcast when a button's description text should be shown or cleared.
	 *
	 * UFrontendUICommonButtonBase fires this on hover/unhover.
	 * Bind in Blueprint or C++ to update a description text block in the UI.
	 */
	UPROPERTY(BlueprintAssignable)
	FOnButtonDescriptionTextUpdatedDelegate OnButtonDescriptionTextUpdated;

private:
	/**
	 * @brief The active primary layout widget registered via RegisterCreatedPrimaryLayoutWidget.
	 * Transient: not serialized, recreated each session when the layout widget is added to the viewport.
	 */
	UPROPERTY(Transient)
	UWidget_PrimaryLayout* CreatedPrimaryLayout;
};
