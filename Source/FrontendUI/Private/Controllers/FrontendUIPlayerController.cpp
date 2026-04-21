#include "Controllers/FrontendUIPlayerController.h"

#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "FrontendUISettings/FrontendUIGameUserSettings.h"

void AFrontendUIPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	// -------------------------------------------------------------------------
	// VIEW TARGET SETUP
	// -------------------------------------------------------------------------
	// Find the scene camera tagged "Default" and set it as the active view target.
	// Using an Actor Tag ("Default") instead of a hard reference allows level
	// designers to swap the camera without modifying C++ code.
	TArray<AActor*> FoundCameras;
	UGameplayStatics::GetAllActorsOfClassWithTag(this,
	                                             ACameraActor::StaticClass(),
	                                             FName("Default"),
	                                             FoundCameras);

	if (!FoundCameras.IsEmpty())
	{
		SetViewTarget(FoundCameras[0]);
	}

	// -------------------------------------------------------------------------
	// HARDWARE BENCHMARK
	// -------------------------------------------------------------------------
	// On first launch, no benchmark results exist (value is -1).
	// Run the benchmark and apply results as the initial video quality settings,
	// giving new players a reasonable default configuration for their hardware.
	// On subsequent launches, benchmark results are already stored in GameUserSettings.ini
	// and this block is skipped entirely.
	UFrontendUIGameUserSettings* GameUserSettings = UFrontendUIGameUserSettings::Get();

	if (!ensureMsgf(GameUserSettings,
	                TEXT(
		                "AFrontendUIPlayerController::OnPossess — GameUserSettings is null. Cannot run hardware benchmark."
	                )))
	{
		return;
	}

	if (GameUserSettings->GetLastCPUBenchmarkResult() == -1.f || GameUserSettings->GetLastGPUBenchmarkResult() == -1.f)
	{
		GameUserSettings->RunHardwareBenchmark();
		GameUserSettings->ApplyHardwareBenchmarkResults();
	}
}
