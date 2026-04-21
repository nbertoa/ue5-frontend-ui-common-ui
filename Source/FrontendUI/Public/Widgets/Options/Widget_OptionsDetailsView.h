#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widget_OptionsDetailsView.generated.h"

/**
 * @class UWidget_OptionsDetailsView
 * @brief Panel widget that displays contextual information about the selected or hovered options entry.
 *
 * Shows the selected setting's display name, description image, rich-text description,
 * dynamic debug details (data object class and entry widget class), and a disabled reason
 * when the setting is locked by an edit condition.
 *
 * @section Architecture Push Model
 * This widget does not fetch data itself — it receives data via UpdateDetailsViewInfo,
 * called by UWidget_OptionsScreen when the list selection or hover state changes.
 * This keeps the details view decoupled from the list view and the data registry.
 *
 * @section Lazy Image Loading
 * The description image uses UCommonLazyImage, which loads the texture asynchronously
 * from a TSoftObjectPtr when SetBrushFromLazyTexture is called. This avoids blocking
 * the game thread when switching between settings with different description images.
 *
 * @section Dynamic Details
 * CommonRichText_DynamicDetails displays the data object class name and the entry widget
 * class name as rich text. This is primarily a development aid — it helps verify that
 * the correct data object type and entry widget are being used for each setting.
 *
 * @section DisableNaiveTick
 * meta = (DisableNaiveTick) prevents UMG from enabling Tick unless explicitly requested.
 * All updates are driven by explicit UpdateDetailsViewInfo / ClearDetailsViewInfo calls.
 *
 * @see UWidget_OptionsScreen for where UpdateDetailsViewInfo and ClearDetailsViewInfo are called.
 * @see UListDataObject_Base for the data object whose metadata this widget displays.
 */
UCLASS(Abstract,
	BlueprintType,
	meta = (DisableNaiveTick))
class FRONTENDUI_API UWidget_OptionsDetailsView : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * @brief Updates all details view fields from the given data object.
	 *
	 * Sets the title, lazily loads the description image (if any), sets the
	 * rich-text description, populates the dynamic details debug text, and
	 * shows or hides the disabled reason based on the data object's current editable state.
	 *
	 * @param InDataObject          The data object to display. If null, does nothing.
	 * @param InEntryWidgetClassName The class name of the entry widget currently displaying this data object.
	 *                              Used for the dynamic details debug text. Defaults to empty string.
	 */
	void UpdateDetailsViewInfo(class UListDataObject_Base* InDataObject,
	                           const FString& InEntryWidgetClassName = FString()) const;

	/**
	 * @brief Clears all details view fields to their empty/hidden state.
	 * Called when no entry is selected or hovered, or when the active tab changes.
	 */
	void ClearDetailsViewInfo();

protected:
	//~ Begin UUserWidget Interface
	/**
	 * @brief Clears the details view on initialization so no stale data is displayed at startup.
	 */
	virtual void NativeOnInitialized() override;
	//~ End UUserWidget Interface

private:
	/** @brief Bound text block displaying the selected setting's display name. Must exist in Blueprint. */
	UPROPERTY(meta = (BindWidget))
	class UCommonTextBlock* CommonTextBlock_Title;

	/**
	 * @brief Bound lazy image displaying the setting's description image.
	 * Loads asynchronously from TSoftObjectPtr when SetBrushFromLazyTexture is called.
	 * Hidden (Collapsed) when the data object has no description image.
	 * Must exist in Blueprint.
	 */
	UPROPERTY(meta = (BindWidget))
	class UCommonLazyImage* CommonLazyImage_DescriptionImage;

	/** @brief Bound rich text block displaying the setting's description. Must exist in Blueprint. */
	UPROPERTY(meta = (BindWidget))
	class UCommonRichTextBlock* CommonRichText_Description;

	/**
	 * @brief Bound rich text block displaying debug details (data object class and entry widget class).
	 * Primarily useful during development to verify correct data object and widget type mapping.
	 * Must exist in Blueprint.
	 */
	UPROPERTY(meta = (BindWidget))
	class UCommonRichTextBlock* CommonRichText_DynamicDetails;

	/**
	 * @brief Bound rich text block displaying the reason a setting is disabled.
	 * Empty when the setting is editable. Populated from FOptionsDataEditConditionDescriptor
	 * when IsDataCurrentlyEditable() returns false.
	 * Must exist in Blueprint.
	 */
	UPROPERTY(meta = (BindWidget))
	class UCommonRichTextBlock* CommonRichText_DisabledReason;
};
