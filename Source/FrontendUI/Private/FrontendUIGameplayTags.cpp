#include "FrontendUIGameplayTags.h"

namespace FrontendUIGameplayTags
{
	// -------------------------------------------------------------------------
	// WIDGET STACKS
	// -------------------------------------------------------------------------
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(FrontendUI_WidgetStack_Modal,
	                               "FrontendUI.WidgetStack.Modal",
	                               "Blocking modal stack. Used for confirmation dialogs that require user input before proceeding.")
	;

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(FrontendUI_WidgetStack_GameMenu,
	                               "FrontendUI.WidgetStack.GameMenu",
	                               "In-game menu stack. Used for pause menus and game overlays.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(FrontendUI_WidgetStack_GameHud,
	                               "FrontendUI.WidgetStack.GameHud",
	                               "In-game HUD stack. Used for gameplay UI elements like health bars and minimaps.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(FrontendUI_WidgetStack_Frontend,
	                               "FrontendUI.WidgetStack.Frontend",
	                               "Frontend stack. Used for title screen and pre-game UI flows.");

	// -------------------------------------------------------------------------
	// WIDGETS
	// -------------------------------------------------------------------------
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(FrontendUI_Widget_PressAnyKeyScreen,
	                               "FrontendUI.Widget.PressAnyKeyScreen",
	                               "Entry point of the frontend flow. Waits for any input before advancing to the main menu.")
	;

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(FrontendUI_Widget_MainMenuScreen,
	                               "FrontendUI.Widget.MainMenuScreen",
	                               "Main menu screen widget. Root navigation hub for the frontend.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(FrontendUI_Widget_OptionsScreen,
	                               "FrontendUI.Widget.OptionsScreen",
	                               "Options screen widget. Hosts the tabbed settings UI with Video, Audio, Gameplay, and Controls tabs.")
	;

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(FrontendUI_Widget_ConfirmScreen,
	                               "FrontendUI.Widget.ConfirmScreen",
	                               "Confirmation dialog widget. Pushed to the modal stack by PushConfirmScreenToModalStackAsync.")
	;

	// -------------------------------------------------------------------------
	// IMAGES
	// -------------------------------------------------------------------------
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(FrontendUI_Image_TestImage,
	                               "FrontendUI.Image.TestImage",
	                               "Test image used in the options details view for development purposes.");
}
