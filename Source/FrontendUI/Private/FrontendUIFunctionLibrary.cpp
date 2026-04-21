#include "FrontendUIFunctionLibrary.h"

#include "FrontendUISettings/FrontendUIDeveloperSettings.h"

TSoftClassPtr<UWidget_ActivatableBase> UFrontendUIFunctionLibrary::GetWidgetClassByTag(FGameplayTag InWidgetTag)
{
	// GetDefault<> returns the CDO of UFrontendUIDeveloperSettings, which holds
	// the values loaded from DefaultGame.ini. No world context is required.
	const UFrontendUIDeveloperSettings* FrontendUIDeveloperSettings = GetDefault<UFrontendUIDeveloperSettings>();

	// Hard failure: a missing tag mapping is a configuration error, not a runtime edge case.
	// The developer must register all widget tags in Project Settings → Frontend UI Settings.
	checkf(FrontendUIDeveloperSettings->WidgetClassByTag.Contains(InWidgetTag),
	       TEXT("UFrontendUIFunctionLibrary::GetWidgetClassByTag — No widget class mapped to tag: %s. "
		       "Register it in Project Settings → Frontend UI Settings → Widget Reference."),
	       *InWidgetTag.ToString());

	return FrontendUIDeveloperSettings->WidgetClassByTag.FindRef(InWidgetTag);
}

TSoftObjectPtr<UTexture2D> UFrontendUIFunctionLibrary::GetOptionsSoftImageByTag(FGameplayTag InImageTag)
{
	const UFrontendUIDeveloperSettings* FrontendUIDeveloperSettings = GetDefault<UFrontendUIDeveloperSettings>();

	// Hard failure: a missing image tag mapping is a configuration error.
	// The developer must register all image tags in Project Settings → Frontend UI Settings.
	checkf(FrontendUIDeveloperSettings->OptionsScreenSoftImageMap.Contains(InImageTag),
	       TEXT("UFrontendUIFunctionLibrary::GetOptionsSoftImageByTag — No image mapped to tag: %s. "
		       "Register it in Project Settings → Frontend UI Settings → Options Image Reference."),
	       *InImageTag.ToString());

	return FrontendUIDeveloperSettings->OptionsScreenSoftImageMap.FindRef(InImageTag);
}
