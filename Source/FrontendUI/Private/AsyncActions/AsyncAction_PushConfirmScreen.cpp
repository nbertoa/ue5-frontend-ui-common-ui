#include "AsyncActions/AsyncAction_PushConfirmScreen.h"

#include "Subsystems/FrontendUISubsystem.h"

UAsyncAction_PushConfirmScreen* UAsyncAction_PushConfirmScreen::PushConfirmScreen(const UObject* InWorldContextObject,
	EConfirmScreenType InScreenType,
	FText InScreenTitle,
	FText InScreenMessage)
{
	if (!GEngine)
	{
		return nullptr;
	}

	// Resolve the world from the context object.
	// LogAndReturnNull: non-fatal — returns nullptr if the world is not available
	// rather than asserting, since this can be called from editor contexts.
	UWorld* World = GEngine->GetWorldFromContextObject(InWorldContextObject,
	                                                   EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	// Create the async action node and cache all parameters for use during Activate().
	UAsyncAction_PushConfirmScreen* Node = NewObject<UAsyncAction_PushConfirmScreen>();
	Node->CachedOwningWorld = World;
	Node->CachedScreenType = InScreenType;
	Node->CachedScreenTitle = InScreenTitle;
	Node->CachedScreenMessage = InScreenMessage;

	// RegisterWithGameInstance ties this node's lifetime to the GameInstance,
	// ensuring it is not garbage collected before Activate() is called or
	// before the user interacts with the confirmation screen.
	Node->RegisterWithGameInstance(World);

	return Node;
}

void UAsyncAction_PushConfirmScreen::Activate()
{
	UFrontendUISubsystem* FrontendUISubsystem = UFrontendUISubsystem::Get(CachedOwningWorld.Get());

	if (!ensureMsgf(FrontendUISubsystem,
	                TEXT("UAsyncAction_PushConfirmScreen::Activate — FrontendUISubsystem is null. "
		                "Ensure the subsystem is created for this GameInstance.")))
	{
		SetReadyToDestroy();
		return;
	}

	FrontendUISubsystem->PushConfirmScreenToModalStackAsync(CachedScreenType,
	                                                        CachedScreenTitle,
	                                                        CachedScreenMessage,
	                                                        [this](EConfirmScreenButtonType ClickedButtonType)
	                                                        {
		                                                        // Broadcast the result to Blueprint output pins.
		                                                        OnButtonClicked.Broadcast(ClickedButtonType);

		                                                        // Mark the node as complete so the async action system
		                                                        // can garbage collect it on the next GC cycle.
		                                                        SetReadyToDestroy();
	                                                        });
}
