#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "FrontendUICommonButtonBase.generated.h"

/**
 * @class UFrontendUICommonButtonBase
 * @brief Project-specific button base class extending UCommonButtonBase.
 *
 * Provides text management, style synchronization, hover description broadcasting,
 * and optional uppercase formatting on top of Common UI's button foundation.
 *
 * @section Architecture Common UI Button Pattern
 * UCommonButtonBase handles input routing, gamepad focus, and style switching
 * automatically. This class extends it with:
 * - A bindable text block (CommonTextBlock_ButtonText) driven by ButtonDisplayText.
 * - Automatic style propagation to the text block via NativeOnCurrentTextStyleChanged.
 * - Hover/unhover events that broadcast description text through UFrontendUISubsystem,
 *   allowing any UI element to display contextual help without coupling to the button.
 *
 * @section BindWidgetOptional
 * CommonTextBlock_ButtonText uses BindWidgetOptional — buttons without a text block
 * (e.g., icon-only buttons) are valid and will simply skip text operations silently.
 *
 * @section DisableNaiveTick
 * meta = (DisableNaiveTick) prevents UMG from enabling Tick unless explicitly requested.
 * All updates are event-driven via Common UI's style and input callbacks.
 *
 * @see UFrontendUISubsystem::OnButtonDescriptionTextUpdated for the description broadcast.
 */
UCLASS(Abstract,
	BlueprintType,
	meta = (DisableNaiveTick))
class FRONTENDUI_API UFrontendUICommonButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	/**
	 * @brief Sets the button's display text programmatically.
	 *
	 * Applies uppercase formatting if bUserUpperCaseForButtonText is true.
	 * Has no effect if CommonTextBlock_ButtonText is not bound or InText is empty.
	 *
	 * @param InText The text to display on the button.
	 */
	UFUNCTION(BlueprintCallable)
	void SetButtonText(FText InText);

	/**
	 * @brief Returns the text currently displayed by the button's text block.
	 *
	 * Returns an empty FText if CommonTextBlock_ButtonText is not bound.
	 *
	 * @return The current display text, or FText::GetEmpty() if no text block is bound.
	 */
	UFUNCTION(BlueprintCallable)
	FText GetButtonDisplayText() const;

private:
	//~ Begin UUserWidget Interface
	/**
	 * @brief Applies ButtonDisplayText to the text block at design time.
	 * Allows the button text to be previewed in the UMG editor without running the game.
	 */
	virtual void NativePreConstruct() override;
	//~ End UUserWidget Interface

	//~ Begin UCommonButtonBase Interface
	/**
	 * @brief Propagates the current Common UI text style to the bound text block.
	 * Called automatically by Common UI when the button's style state changes
	 * (e.g., normal → hovered → pressed → disabled).
	 */
	virtual void NativeOnCurrentTextStyleChanged() override;

	/**
	 * @brief Broadcasts the button's description text via UFrontendUISubsystem on hover.
	 * Allows any UI element (e.g., a description panel) to display contextual help
	 * without being directly coupled to this button.
	 */
	virtual void NativeOnHovered() override;

	/**
	 * @brief Broadcasts an empty description text via UFrontendUISubsystem on unhover.
	 * Signals listeners to clear the description display.
	 */
	virtual void NativeOnUnhovered() override;
	//~ End UCommonButtonBase Interface

	/**
	 * @brief Optional bound text block for the button label.
	 * BindWidgetOptional: buttons without a text block (e.g., icon-only) are valid.
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	class UCommonTextBlock* CommonTextBlock_ButtonText;

	/**
	 * @brief Default display text for this button, configurable per instance in the editor.
	 * Applied to CommonTextBlock_ButtonText during NativePreConstruct and SetButtonText.
	 */
	UPROPERTY(EditAnywhere,
		BlueprintReadOnly,
		Category = "Frontend UI Button",
		meta = (AllowPrivateAccess = "true"))
	FText ButtonDisplayText;

	/**
	 * @brief If true, button text is converted to uppercase before display.
	 * Useful for stylistic consistency without requiring uppercase source strings.
	 */
	UPROPERTY(EditAnywhere,
		BlueprintReadOnly,
		Category = "Frontend UI Button",
		meta = (AllowPrivateAccess = "true"))
	bool bUserUpperCaseForButtonText = false;

	/**
	 * @brief Contextual description text broadcast on hover via UFrontendUISubsystem.
	 * Empty by default — buttons without descriptions simply broadcast nothing on hover.
	 */
	UPROPERTY(EditAnywhere,
		BlueprintReadOnly,
		Category = "Frontend UI Button",
		meta = (AllowPrivateAccess = "true"))
	FText ButtonDescriptionText;
};
