#pragma once

#include "CoreMinimal.h"
#include "Widgets/Widget_ActivatableBase.h"
#include "FrontendUITypes/FrontendUIEnumTypes.h"
#include "Widget_ConfirmScreen.generated.h"

/**
 * @brief Data transfer object carrying all information needed to populate a confirmation screen.
 *
 * Created by the static factory functions (CreateOKScreen, CreateYesNoScreen, CreateOkCancelScreen)
 * and passed to UWidget_ConfirmScreen::InitConfirmScreen before the widget becomes visible.
 * Using a separate info object decouples the screen layout from the data that drives it,
 * allowing the same widget Blueprint to display different button configurations at runtime.
 *
 * Marked Transient: not serialized — created and consumed within a single push/pop cycle.
 */
USTRUCT(BlueprintType)
struct FConfirmScreenButtonInfo
{
	GENERATED_BODY()

	/** @brief The semantic type of this button (Confirmed, Cancelled, Closed). */
	UPROPERTY(EditAnywhere,
		BlueprintReadWrite)
	EConfirmScreenButtonType ConfirmScreenButtonType = EConfirmScreenButtonType::Unknown;

	/** @brief The localized text displayed on this button. */
	UPROPERTY(EditAnywhere,
		BlueprintReadWrite)
	FText ButtonTextToDisplay;
};

/**
 * @class UConfirmScreenInfoObject
 * @brief Data transfer object that carries title, message, and button configuration for a confirmation screen.
 *
 * Created via static factory functions and passed to UWidget_ConfirmScreen::InitConfirmScreen
 * before the widget is pushed to the modal stack. This separates screen configuration
 * from the widget's visual implementation.
 *
 * @see UFrontendUISubsystem::PushConfirmScreenToModalStackAsync
 */
UCLASS()
class FRONTENDUI_API UConfirmScreenInfoObject : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief Creates an info object for a single-button acknowledgment screen.
	 * The button uses EConfirmScreenButtonType::Closed and the default back input action.
	 *
	 * @param InScreenTitle Title text displayed at the top of the screen.
	 * @param InScreenMsg   Body message text displayed on the screen.
	 * @return Configured UConfirmScreenInfoObject with one "Ok" button.
	 */
	static UConfirmScreenInfoObject* CreateOKScreen(const FText& InScreenTitle,
	                                                const FText& InScreenMsg);

	/**
	 * @brief Creates an info object for a binary choice screen.
	 * Buttons: "Yes" (Confirmed) and "No" (Cancelled).
	 * Focus defaults to "No" to prevent accidental confirmation of destructive actions.
	 *
	 * @param InScreenTitle Title text displayed at the top of the screen.
	 * @param InScreenMsg   Body message text displayed on the screen.
	 * @return Configured UConfirmScreenInfoObject with "Yes" and "No" buttons.
	 */
	static UConfirmScreenInfoObject* CreateYesNoScreen(const FText& InScreenTitle,
	                                                   const FText& InScreenMsg);

	/**
	 * @brief Creates an info object for a neutral confirmation screen.
	 * Buttons: "Ok" (Confirmed) and "Cancel" (Cancelled).
	 *
	 * @param InScreenTitle Title text displayed at the top of the screen.
	 * @param InScreenMsg   Body message text displayed on the screen.
	 * @return Configured UConfirmScreenInfoObject with "Ok" and "Cancel" buttons.
	 */
	static UConfirmScreenInfoObject* CreateOkCancelScreen(const FText& InScreenTitle,
	                                                      const FText& InScreenMsg);

	/** @brief Title text displayed at the top of the confirmation screen. */
	UPROPERTY(Transient)
	FText ScreenTitle;

	/** @brief Body message text displayed on the confirmation screen. */
	UPROPERTY(Transient)
	FText ScreenMessage;

	/** @brief Ordered list of buttons to display. Order determines layout and initial focus. */
	UPROPERTY(Transient)
	TArray<FConfirmScreenButtonInfo> AvailableScreenButtons;
};

/**
 * @class UWidget_ConfirmScreen
 * @brief Modal confirmation dialog that displays a title, message, and dynamically generated buttons.
 *
 * Pushed onto the modal widget stack by UFrontendUISubsystem::PushConfirmScreenToModalStackAsync.
 * Button layout and behavior are driven by a UConfirmScreenInfoObject passed via InitConfirmScreen
 * before the widget becomes visible (OnCreatedBeforePush phase).
 *
 * @section Architecture Dynamic Button Generation
 * Buttons are created at runtime using UDynamicEntryBox, which instantiates
 * UFrontendUICommonButtonBase entries on demand. This allows the same widget Blueprint
 * to display 1, 2, or more buttons without requiring separate widget classes per configuration.
 *
 * Each button is bound to a lambda that calls the external callback and deactivates the widget,
 * keeping the confirm screen decoupled from the system that requested it.
 *
 * @section Focus Management
 * NativeGetDesiredFocusTarget returns the last button entry, which is intentionally
 * the "negative" option (No/Cancel) for destructive action dialogs — preventing
 * accidental confirmation via gamepad/keyboard.
 *
 * @see UConfirmScreenInfoObject for button configuration.
 * @see UFrontendUISubsystem::PushConfirmScreenToModalStackAsync for the push flow.
 */
UCLASS(Abstract,
	BlueprintType,
	meta = (DisableNaiveTick))
class FRONTENDUI_API UWidget_ConfirmScreen : public UWidget_ActivatableBase
{
	GENERATED_BODY()

public:
	/**
	 * @brief Initializes the screen with its data and button callback.
	 *
	 * Must be called during the OnCreatedBeforePush phase, before the widget
	 * becomes visible in the modal stack. Populates title, message, and
	 * dynamically generates buttons from the info object's button array.
	 *
	 * @param InScreenInfoObject     Data object carrying title, message, and button config.
	 * @param InClickedButtonCallback Called with the button type when the user clicks a button.
	 */
	void InitConfirmScreen(UConfirmScreenInfoObject* InScreenInfoObject,
	                       TFunction<void(EConfirmScreenButtonType)> InClickedButtonCallback);

private:
	/**
	 * @brief Returns the last button entry as the desired focus target.
	 *
	 * The last button is intentionally the "negative" option (No/Cancel)
	 * to prevent accidental confirmation of destructive actions via gamepad/keyboard.
	 */
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	/** @brief Bound widget displaying the screen title. Must exist in the Blueprint subclass. */
	UPROPERTY(meta = (BindWidget))
	class UCommonTextBlock* CommonTextBlock_Title;

	/** @brief Bound widget displaying the body message. Must exist in the Blueprint subclass. */
	UPROPERTY(meta = (BindWidget))
	UCommonTextBlock* CommonTextBlock_Message;

	/**
	 * @brief Bound widget that generates button entries dynamically at runtime.
	 * Entry widget class is configured in the Blueprint subclass.
	 * Must exist in the Blueprint subclass.
	 */
	UPROPERTY(meta = (BindWidget))
	class UDynamicEntryBox* DynamicEntryBox_Buttons;
};
