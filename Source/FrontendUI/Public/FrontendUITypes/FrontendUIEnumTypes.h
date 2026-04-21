#pragma once

#include "CoreMinimal.h"

/**
 * @brief Defines the visual and behavioral layout of a confirmation screen.
 *
 * Used by UFrontendUISubsystem::PushConfirmScreenToModalStackAsync to determine
 * which buttons to display and how to configure the UWidget_ConfirmScreen instance.
 */
UENUM(BlueprintType)
enum class EConfirmScreenType : uint8
{
	/** Single acknowledgment button. Used for informational dialogs with no binary choice. */
	Ok,

	/** Two-option choice screen. Used for destructive actions requiring explicit confirmation. */
	YesNo,

	/** Two-option screen where one option is neutral/cancellable. Used for non-destructive confirmations. */
	OKCancel,

	/** Fallback value. Should never be used intentionally — indicates uninitialized state. */
	Unknown UMETA(Hidden)
};

/**
 * @brief Identifies which button the user clicked on a confirmation screen.
 *
 * Passed as a payload to the TFunction callback registered via
 * UFrontendUISubsystem::PushConfirmScreenToModalStackAsync, allowing
 * the caller to react to the user's choice without coupling to the widget.
 */
UENUM(BlueprintType)
enum class EConfirmScreenButtonType : uint8
{
	/** The user accepted or confirmed the action (e.g., "Yes", "OK"). */
	Confirmed,

	/** The user explicitly rejected the action (e.g., "No", "Cancel"). */
	Cancelled,

	/** The user dismissed the screen without making a choice (e.g., back button on an OK screen). */
	Closed,

	/** Fallback value. Should never be used intentionally — indicates uninitialized state. */
	Unknown UMETA(Hidden)
};

/**
 * @brief Describes why a list data object was modified.
 *
 * Passed alongside the modified data object in OnListDataModified broadcasts,
 * allowing listeners (widgets, registries) to react differently based on
 * the cause of the change — direct user input, a dependency change, or a reset.
 */
UENUM(BlueprintType)
enum class EOptionsListDataModifyReason : uint8
{
	/** The user directly modified this setting (e.g., moved a slider, selected a new option). */
	DirectlyModified,

	/** This setting changed because a setting it depends on changed (e.g., resolution locked by fullscreen mode). */
	DependencyModified,

	/** The setting was reset to its default value via the Reset action. */
	ResetToDefault
};
