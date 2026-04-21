#include "Widgets/Options/DataObjects/ListDataObject_StringResolution.h"

#include "Kismet/KismetSystemLibrary.h"
#include "FrontendUISettings/FrontendUIGameUserSettings.h"

void UListDataObject_StringResolution::InitResolutionValues()
{
	TArray<FIntPoint> AvailableResolutions;

	// Query all resolutions supported by the platform for fullscreen mode.
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(AvailableResolutions);

	// Sort from lowest to highest by total pixel count (SizeSquared = X*X + Y*Y).
	// This presents resolutions in a natural ascending order in the UI.
	AvailableResolutions.Sort([](const FIntPoint& A,
	                             const FIntPoint& B) -> bool
	{
		return A.SizeSquared() < B.SizeSquared();
	});

	for (const FIntPoint& Resolution : AvailableResolutions)
	{
		AddDynamicOption(ResToValueString(Resolution),
		                 ResToDisplayText(Resolution));
	}

	// Cache the maximum resolution — used by UOptionsDataRegistry to configure
	// the window mode dependency condition (force to max when not fullscreen).
	MaximumAllowedResolution = ResToValueString(AvailableResolutions.Last());

	// Set the maximum resolution as the default value for reset support.
	SetDefaultValueFromString(MaximumAllowedResolution);
}

void UListDataObject_StringResolution::OnDataObjectInitialized()
{
	Super::OnDataObjectInitialized();

	// Fallback: if the saved resolution is no longer in the supported list
	// (e.g., after a monitor change or driver update), display the current
	// engine resolution instead of "Invalid Option".
	if (!TrySetDisplayTextFromStringValue(CurrentStringValue))
	{
		CurrentDisplayText = ResToDisplayText(UFrontendUIGameUserSettings::Get()->GetScreenResolution());
	}
}

FString UListDataObject_StringResolution::ResToValueString(const FIntPoint& InResolution)
{
	// Format matches what UFrontendUIGameUserSettings::GetScreenResolution returns
	// when converted to string via the dynamic getter, ensuring correct comparison.
	return FString::Printf(TEXT("(X=%i,Y=%i)"),
	                       InResolution.X,
	                       InResolution.Y);
}

FText UListDataObject_StringResolution::ResToDisplayText(const FIntPoint& InResolution)
{
	// User-friendly format: "1920 x 1080".
	const FString DisplayString = FString::Printf(TEXT("%i x %i"),
	                                              InResolution.X,
	                                              InResolution.Y);

	return FText::FromString(DisplayString);
}
