#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "FrontendUIGameUserSettings.generated.h"

/**
 * @class UFrontendUIGameUserSettings
 * @brief Custom GameUserSettings extension for project-specific player preferences.
 *
 * Extends the engine's UGameUserSettings to add game-specific settings categories:
 * - Gameplay: difficulty selection.
 * - Audio: volume controls (overall, music, SFX) and audio mode flags.
 * - Video: display gamma.
 *
 * All properties are marked UPROPERTY(Config) so they persist to the
 * GameUserSettings.ini file automatically via the engine's save/load pipeline.
 *
 * @section Usage
 * Always access via the static Get() helper instead of GEngine->GetGameUserSettings()
 * to get the correctly typed pointer without manual casting at every call site.
 *
 * @see UOptionsDataRegistry for how these settings are exposed to the options screen.
 * @see FOptionsDataInteractionHelper for how getters/setters are bound dynamically.
 */
UCLASS()
class FRONTENDUI_API UFrontendUIGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UFrontendUIGameUserSettings();

	/**
	 * @brief Type-safe accessor for the game's custom user settings instance.
	 *
	 * Retrieves and casts GEngine->GetGameUserSettings() to this class.
	 * Avoids manual CastChecked at every call site.
	 *
	 * @return Pointer to the active UFrontendUIGameUserSettings instance, or nullptr if GEngine is invalid.
	 */
	static UFrontendUIGameUserSettings* Get();

	// -------------------------------------------------------------------------
	// GAMEPLAY SETTINGS
	// -------------------------------------------------------------------------

	/**
	 * @brief Returns the currently selected game difficulty as a string.
	 * The string value maps to a difficulty option registered in UOptionsDataRegistry.
	 */
	UFUNCTION()
	FString GetCurrentGameDifficulty() const { return CurrentGameDifficulty; }

	/**
	 * @brief Sets the current game difficulty.
	 * @param InNewDifficulty The difficulty string to apply (must match a registered option).
	 */
	UFUNCTION()
	void SetCurrentGameDifficulty(const FString& InNewDifficulty) { CurrentGameDifficulty = InNewDifficulty; }

	// -------------------------------------------------------------------------
	// AUDIO SETTINGS
	// -------------------------------------------------------------------------

	/** @brief Returns the overall/master volume level (0.0 - 1.0). */
	UFUNCTION()
	float GetOverallVolume() const { return OverallVolume; }

	/**
	 * @brief Sets the overall/master volume level.
	 * Implement audio bus or Sound Mix modifier logic here.
	 * @param InVolume New volume value (0.0 - 1.0).
	 */
	UFUNCTION()
	void SetOverallVolume(float InVolume);

	/** @brief Returns the music volume level (0.0 - 1.0). */
	UFUNCTION()
	float GetMusicVolume() const { return MusicVolume; }

	/**
	 * @brief Sets the music volume level.
	 * @param InVolume New volume value (0.0 - 1.0).
	 */
	UFUNCTION()
	void SetMusicVolume(float InVolume);

	/** @brief Returns the sound effects volume level (0.0 - 1.0). */
	UFUNCTION()
	float GetSoundFXVolume() const { return SoundFXVolume; }

	/**
	 * @brief Sets the sound effects volume level.
	 * @param InVolume New volume value (0.0 - 1.0).
	 */
	UFUNCTION()
	void SetSoundFXVolume(float InVolume);

	/** @brief Returns whether audio should continue playing when the application loses focus. */
	UFUNCTION()
	bool GetAllowBackgroundAudio() const { return bAllowBackgroundAudio; }

	/**
	 * @brief Sets whether background audio playback is allowed.
	 * @param bIsAllowed True to allow audio when the app is not in focus.
	 */
	UFUNCTION()
	void SetAllowBackgroundAudio(bool bIsAllowed);

	/** @brief Returns whether HDR audio mode is enabled. */
	UFUNCTION()
	bool GetUseHDRAudioMode() const { return bUseHDRAudioMode; }

	/**
	 * @brief Sets whether HDR audio mode is enabled.
	 * @param bIsAllowed True to enable HDR audio processing.
	 */
	UFUNCTION()
	void SetUseHDRAudioMode(bool bIsAllowed);

	// -------------------------------------------------------------------------
	// VIDEO SETTINGS
	// -------------------------------------------------------------------------

	/**
	 * @brief Returns the current display gamma value from GEngine.
	 *
	 * Reads directly from GEngine->DisplayGamma rather than a cached property,
	 * ensuring the returned value always reflects the active engine state.
	 *
	 * @return Current display gamma, or 0.0 if GEngine is invalid.
	 */
	UFUNCTION()
	static float GetCurrentDisplayGamma();

	/**
	 * @brief Sets the display gamma directly on GEngine.
	 *
	 * Bypasses the standard Config property pipeline because GEngine->DisplayGamma
	 * is the authoritative runtime value for gamma correction.
	 *
	 * @param InNewGamma New gamma value to apply.
	 */
	UFUNCTION()
	void SetCurrentDisplayGamma(float InNewGamma);

private:
	// -------------------------------------------------------------------------
	// GAMEPLAY PROPERTIES
	// -------------------------------------------------------------------------

	/**
	 * @brief Persisted difficulty selection.
	 * Stored as FString to remain decoupled from a specific enum or data table.
	 */
	UPROPERTY(Config)
	FString CurrentGameDifficulty;

	// -------------------------------------------------------------------------
	// AUDIO PROPERTIES
	// -------------------------------------------------------------------------

	/** @brief Master/overall volume. Persisted via Config. Default: 1.0 (full volume). */
	UPROPERTY(Config)
	float OverallVolume;

	/** @brief Music channel volume. Persisted via Config. Default: 1.0. */
	UPROPERTY(Config)
	float MusicVolume;

	/** @brief Sound effects channel volume. Persisted via Config. Default: 1.0. */
	UPROPERTY(Config)
	float SoundFXVolume;

	/** @brief If true, audio continues when the application loses focus. Default: false. */
	UPROPERTY(Config)
	bool bAllowBackgroundAudio;

	/** @brief If true, HDR audio processing is enabled. Default: false. */
	UPROPERTY(Config)
	bool bUseHDRAudioMode;
};
