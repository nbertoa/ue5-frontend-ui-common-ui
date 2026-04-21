#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * @file FrontendUIGameplayTags.h
 * @brief Centralized declaration of Native Gameplay Tags for the FrontendUI module.
 *
 * Native Gameplay Tags allow C++ systems to reference tags without unsafe string literals.
 * All tags are organized by category using a flat namespace that mirrors the tag hierarchy.
 *
 * Tag categories:
 * - WidgetStack: Identifies named widget stacks registered in UWidget_PrimaryLayout.
 * - Widget:      Identifies specific screen widgets mapped in UFrontendUIDeveloperSettings.
 * - Image:       Identifies images mapped in UFrontendUIDeveloperSettings for the options screen.
 *
 * @see UFrontendUIDeveloperSettings
 * @see UWidget_PrimaryLayout
 * @see UFrontendUIFunctionLibrary
 */
namespace FrontendUIGameplayTags
{
	// -------------------------------------------------------------------------
	// WIDGET STACKS
	// Tags used to identify named UCommonActivatableWidgetContainerBase instances
	// registered in UWidget_PrimaryLayout. Each stack has a specific purpose:
	// - Modal:    Blocking screens that require user input before proceeding (e.g., confirmations).
	// - GameMenu: In-game pause/menu overlays.
	// - GameHud:  In-game HUD layer (health bars, minimaps, etc.).
	// - Frontend: Title screen / pre-game UI layer.
	// -------------------------------------------------------------------------

	/** @brief Tag for the modal widget stack. Used for confirmation and blocking dialogs. */
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(FrontendUI_WidgetStack_Modal);

	/** @brief Tag for the in-game menu widget stack. Used for pause menus and game overlays. */
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(FrontendUI_WidgetStack_GameMenu);

	/** @brief Tag for the in-game HUD widget stack. Used for gameplay UI elements. */
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(FrontendUI_WidgetStack_GameHud);

	/** @brief Tag for the frontend widget stack. Used for title screen and pre-game UI. */
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(FrontendUI_WidgetStack_Frontend);

	// -------------------------------------------------------------------------
	// WIDGETS
	// Tags used as keys in UFrontendUIDeveloperSettings::WidgetClassByTag.
	// UFrontendUIFunctionLibrary::GetWidgetClassByTag resolves these tags
	// to their corresponding TSoftClassPtr<UWidget_ActivatableBase> at runtime.
	// -------------------------------------------------------------------------

	/** @brief Tag identifying the Press Any Key screen widget. Entry point of the frontend flow. */
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(FrontendUI_Widget_PressAnyKeyScreen);

	/** @brief Tag identifying the Main Menu screen widget. */
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(FrontendUI_Widget_MainMenuScreen);

	/** @brief Tag identifying the Options screen widget. */
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(FrontendUI_Widget_OptionsScreen);

	/** @brief Tag identifying the Confirm screen widget. Used by PushConfirmScreenToModalStackAsync. */
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(FrontendUI_Widget_ConfirmScreen);

	// -------------------------------------------------------------------------
	// IMAGES
	// Tags used as keys in UFrontendUIDeveloperSettings::OptionsScreenSoftImageMap.
	// UFrontendUIFunctionLibrary::GetOptionsSoftImageByTag resolves these tags
	// to their corresponding TSoftObjectPtr<UTexture2D> at runtime.
	// -------------------------------------------------------------------------

	/** @brief Tag identifying the test image used in the options details view. */
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(FrontendUI_Image_TestImage);
}
