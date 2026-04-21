#include "FrontendUISettings/FrontendUIGameUserSettings.h"

UFrontendUIGameUserSettings::UFrontendUIGameUserSettings()
	: OverallVolume(1.0f), MusicVolume(1.0f), SoundFXVolume(1.0f), bAllowBackgroundAudio(false), bUseHDRAudioMode(false)
{
}

UFrontendUIGameUserSettings* UFrontendUIGameUserSettings::Get()
{
	if (GEngine)
	{
		// CastChecked is appropriate here: if GEngine is valid and the project
		// has correctly set DefaultEngine.ini to use this class, the cast
		// must succeed. A failure indicates a project configuration error.
		return CastChecked<UFrontendUIGameUserSettings>(GEngine->GetGameUserSettings());
	}

	return nullptr;
}

void UFrontendUIGameUserSettings::SetOverallVolume(float InVolume)
{
	OverallVolume = InVolume;

	// TODO: Apply the new volume to the active Sound Mix or Audio Bus here.
	// Example: UAudioMixerBlueprintLibrary::SetSoundMixClassOverride(...)
}

void UFrontendUIGameUserSettings::SetMusicVolume(float InVolume)
{
	MusicVolume = InVolume;

	// TODO: Apply the new volume to the music audio bus or Sound Class here.
}

void UFrontendUIGameUserSettings::SetSoundFXVolume(float InVolume)
{
	SoundFXVolume = InVolume;

	// TODO: Apply the new volume to the SFX audio bus or Sound Class here.
}

void UFrontendUIGameUserSettings::SetAllowBackgroundAudio(bool bIsAllowed)
{
	bAllowBackgroundAudio = bIsAllowed;

	// TODO: Toggle background audio behavior via platform audio settings here.
}

void UFrontendUIGameUserSettings::SetUseHDRAudioMode(bool bIsAllowed)
{
	bUseHDRAudioMode = bIsAllowed;

	// TODO: Switch audio processing pipeline to HDR mode here if supported.
}

float UFrontendUIGameUserSettings::GetCurrentDisplayGamma()
{
	if (GEngine)
	{
		// Read directly from GEngine — this is the authoritative runtime value,
		// not a cached property, ensuring the UI always reflects the active state.
		return GEngine->GetDisplayGamma();
	}

	return 0.0f;
}

void UFrontendUIGameUserSettings::SetCurrentDisplayGamma(float InNewGamma)
{
	if (GEngine)
	{
		// Set directly on GEngine instead of going through the Config pipeline,
		// because DisplayGamma is a runtime engine value that takes effect immediately.
		GEngine->DisplayGamma = InNewGamma;
	}
}
