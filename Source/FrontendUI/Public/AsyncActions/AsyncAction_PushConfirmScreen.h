#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "FrontendUITypes/FrontendUIEnumTypes.h"
#include "AsyncAction_PushConfirmScreen.generated.h"

/**
 * @brief Delegate fired when the user clicks a button on the confirmation screen.
 *
 * @param ClickedButtonType Identifies which button was clicked (Confirmed, Cancelled, Closed).
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConfirmScreenButtonClickedDelegate,
                                            EConfirmScreenButtonType,
                                            ClickedButtonType);

/**
 * @class UAsyncAction_PushConfirmScreen
 * @brief Async Blueprint node that pushes a typed confirmation screen onto the modal stack.
 *
 * Wraps UFrontendUISubsystem::PushConfirmScreenToModalStackAsync in a
 * UBlueprintAsyncActionBase so Blueprint graphs can react to button clicks
 * via output execution pins without blocking the game thread.
 *
 * @section Lifecycle
 * 1. Blueprint calls PushConfirmScreen (factory function) → node activates.
 * 2. The subsystem async-loads and pushes the confirm screen widget.
 * 3. When the user clicks a button → OnButtonClicked fires with the button type.
 * 4. The node calls SetReadyToDestroy() and is garbage collected.
 *
 * @section Usage
 * Use this node when you need a blocking confirmation dialog from Blueprint
 * (e.g., "Are you sure you want to quit?") with a clean output-pin-based flow.
 *
 * @see UFrontendUISubsystem::PushConfirmScreenToModalStackAsync
 * @see EConfirmScreenType for available screen layouts.
 * @see EConfirmScreenButtonType for possible button outcomes.
 */
UCLASS()
class FRONTENDUI_API UAsyncAction_PushConfirmScreen : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * @brief Factory function that creates and registers the async action node.
	 *
	 * BlueprintInternalUseOnly ensures this appears as an async node with output
	 * execution pins rather than a regular function in the Blueprint palette.
	 * WorldContext is required to resolve the FrontendUISubsystem.
	 *
	 * @param InWorldContextObject Any UObject with a valid world (e.g., the calling widget).
	 * @param InScreenType         Layout of the confirmation screen (Ok, YesNo, OKCancel).
	 * @param InScreenTitle        Title text displayed at the top of the screen.
	 * @param InScreenMessage      Body message text displayed on the screen.
	 * @return The initialized async action node, or nullptr if the world could not be resolved.
	 */
	UFUNCTION(BlueprintCallable,
		meta = (WorldContext = "InWorldContextObject", HidePin = "WorldContextObject", BlueprintInternalUseOnly = "true"
			, DisplayName = "Show Confirmation Screen"))
	static UAsyncAction_PushConfirmScreen* PushConfirmScreen(const UObject* InWorldContextObject,
	                                                         EConfirmScreenType InScreenType,
	                                                         FText InScreenTitle,
	                                                         FText InScreenMessage);

	//~ Begin UBlueprintAsyncActionBase Interface
	/** @brief Activates the node and initiates the async widget push via UFrontendUISubsystem. */
	virtual void Activate() override;
	//~ End UBlueprintAsyncActionBase Interface

	/**
	 * @brief Fires when the user clicks any button on the confirmation screen.
	 * Exposed as an output execution pin on the Blueprint async node.
	 */
	UPROPERTY(BlueprintAssignable)
	FOnConfirmScreenButtonClickedDelegate OnButtonClicked;

private:
	/** @brief Cached world reference used to retrieve the subsystem during Activate(). */
	TWeakObjectPtr<UWorld> CachedOwningWorld;

	/** @brief The type of confirmation screen to display (Ok, YesNo, OKCancel). */
	EConfirmScreenType CachedScreenType;

	/** @brief Title text to display on the confirmation screen. */
	FText CachedScreenTitle;

	/** @brief Body message text to display on the confirmation screen. */
	FText CachedScreenMessage;
};
