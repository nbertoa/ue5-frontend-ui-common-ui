#include "AsyncActions/AsyncAction_PushSoftWidget.h"

#include "Subsystems/FrontendUISubsystem.h"
#include "Widgets/Widget_ActivatableBase.h"

UAsyncAction_PushSoftWidget* UAsyncAction_PushSoftWidget::PushSoftWidget(const UObject* InWorldContextObject,
                                                                         APlayerController* InOwningPlayerController,
                                                                         TSoftClassPtr<UWidget_ActivatableBase>
                                                                         InSoftWidgetClass,
                                                                         UPARAM(
	                                                                         meta = (Categories =
		                                                                         "FrontendUI.WidgetStack")) FGameplayTag
                                                                         InWidgetStackTag,
                                                                         bool bInFocusOnNewlyPushedWidget)
{
	// Hard failure: a null soft class is a programming error — the caller must
	// always provide a valid soft reference before calling this node.
	checkf(!InSoftWidgetClass.IsNull(),
	       TEXT("UAsyncAction_PushSoftWidget::PushSoftWidget — InSoftWidgetClass is null. "
		       "Ensure a valid soft widget class is provided before calling this node."));

	if (!GEngine)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(InWorldContextObject,
	                                                   EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	// Create the async action node, cache all parameters, and register with the GameInstance
	// to keep the node alive until Activate() completes and SetReadyToDestroy() is called.
	UAsyncAction_PushSoftWidget* Node = NewObject<UAsyncAction_PushSoftWidget>();
	Node->CachedOwningWorld = World;
	Node->CachedOwningPC = InOwningPlayerController;
	Node->CachedSoftWidgetClass = InSoftWidgetClass;
	Node->CachedWidgetStackTag = InWidgetStackTag;
	Node->bCachedFocusOnNewlyPushedWidget = bInFocusOnNewlyPushedWidget;
	Node->RegisterWithGameInstance(World);

	return Node;
}

void UAsyncAction_PushSoftWidget::Activate()
{
	UFrontendUISubsystem* FrontendUISubsystem = UFrontendUISubsystem::Get(CachedOwningWorld.Get());

	if (!ensureMsgf(FrontendUISubsystem,
	                TEXT("UAsyncAction_PushSoftWidget::Activate — FrontendUISubsystem is null. "
		                "Ensure the subsystem is created for this GameInstance.")))
	{
		SetReadyToDestroy();
		return;
	}

	FrontendUISubsystem->PushSoftWidgetToStackAsync(CachedWidgetStackTag,
	                                                CachedSoftWidgetClass,
	                                                [this](EAsyncPushWidgetState InPushState,
	                                                       UWidget_ActivatableBase* PushedWidget)
	                                                {
		                                                switch (InPushState)
		                                                {
		                                                case EAsyncPushWidgetState::OnCreatedBeforePush:
			                                                // Set the owning player before the widget becomes visible,
			                                                // ensuring input routing is correctly configured from the start.
			                                                PushedWidget->SetOwningPlayer(CachedOwningPC.Get());
			                                                OnWidgetCreatedBeforePush.Broadcast(PushedWidget);
			                                                break;

		                                                case EAsyncPushWidgetState::AfterPush:
			                                                AfterPush.Broadcast(PushedWidget);

			                                                // Optionally set focus to the widget's desired focus target.
			                                                // This ensures gamepad/keyboard navigation starts on the correct widget.
			                                                if (bCachedFocusOnNewlyPushedWidget)
			                                                {
				                                                if (UWidget* WidgetToFocus = PushedWidget->
						                                                GetDesiredFocusTarget())
				                                                {
					                                                WidgetToFocus->SetFocus();
				                                                }
			                                                }

			                                                // Mark the node complete — no further callbacks expected.
			                                                SetReadyToDestroy();
			                                                break;

		                                                default:
			                                                break;
		                                                }
	                                                });
}
