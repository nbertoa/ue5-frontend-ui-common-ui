#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FrontendUIPlayerController.generated.h"

/**
 * @class AFrontendUIPlayerController
 * @brief Player Controller for the frontend (title screen) game mode.
 *
 * Handles two responsibilities on possession:
 * 1. View target setup — finds the scene camera tagged "Default" and sets it
 *    as the active view target, giving the frontend its cinematic camera angle.
 * 2. Hardware benchmark — runs the engine's hardware benchmark on first launch
 *    (when no benchmark results exist) and applies the results as default
 *    video settings, giving new players a reasonable starting configuration.
 *
 * @section Architecture
 * This controller owns no input bindings — the frontend UI is driven entirely
 * by Common UI's input routing system via UCommonActivatableWidget and
 * UCommonUIActionRouterBase. The controller's role is purely setup/initialization.
 */
UCLASS()
class FRONTENDUI_API AFrontendUIPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	/**
	 * @brief Called when this controller possesses a Pawn.
	 *
	 * Performs frontend-specific initialization:
	 * - Sets the view target to the scene camera tagged "Default".
	 * - Runs the hardware benchmark if no previous results exist,
	 *   then applies the results as initial video settings.
	 *
	 * @param aPawn The Pawn being possessed (may be a dummy/invisible Pawn in frontend).
	 */
	virtual void OnPossess(APawn* aPawn) override;
};
