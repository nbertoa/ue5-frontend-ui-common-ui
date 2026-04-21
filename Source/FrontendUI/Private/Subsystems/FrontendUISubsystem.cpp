#include "Subsystems/FrontendUISubsystem.h"

#include "Engine/AssetManager.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Widgets/Widget_ActivatableBase.h"
#include "Widgets/Widget_PrimaryLayout.h"
#include "Widgets/Widget_ConfirmScreen.h"
#include "FrontendUIGameplayTags.h"
#include "FrontendUIFunctionLibrary.h"

DEFINE_LOG_CATEGORY(LogFrontendUISubsystem);

UFrontendUISubsystem* UFrontendUISubsystem::Get(const UObject* WorldContextObject)
{
	if (GEngine)
	{
		// Assert mode: a null world context here is a programming error — callers
		// must ensure they have a valid world before calling Get().
		UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,
		                                                   EGetWorldErrorMode::Assert);
		return UGameInstance::GetSubsystem<UFrontendUISubsystem>(World->GetGameInstance());
	}

	return nullptr;
}

bool UFrontendUISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Do not create on Dedicated Servers — frontend UI is client-only.
	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		// If a derived class of this subsystem exists in the project,
		// skip creating the base class to avoid duplicate subsystem instances.
		// The derived class will be created instead.
		TArray<UClass*> FoundClasses;
		GetDerivedClasses(GetClass(),
		                  FoundClasses);

		return FoundClasses.IsEmpty();
	}

	return false;
}

void UFrontendUISubsystem::RegisterCreatedPrimaryLayoutWidget(UWidget_PrimaryLayout* InCreatedWidget)
{
	check(InCreatedWidget);

	CreatedPrimaryLayout = InCreatedWidget;

	UE_LOG(LogFrontendUISubsystem,
	       Log,
	       TEXT("Primary Layout Widget registered: %s"),
	       *InCreatedWidget->GetClass()->GetName());
}

void UFrontendUISubsystem::PushSoftWidgetToStackAsync(const FGameplayTag& InWidgetStackTag,
                                                      TSoftClassPtr<UWidget_ActivatableBase> InSoftWidgetClass,
                                                      TFunction<void(EAsyncPushWidgetState,
                                                                     UWidget_ActivatableBase*)>
                                                      InAsyncPushStateCallback) const
{
	check(!InSoftWidgetClass.IsNull());

	// Request async load of the widget class via AssetManager's StreamableManager.
	// Using async loading avoids stalling the game thread for large widget Blueprints.
	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(InSoftWidgetClass.ToSoftObjectPath(),
	                                                             FStreamableDelegate::CreateLambda(
		                                                             [InSoftWidgetClass, this, InWidgetStackTag,
			                                                             InAsyncPushStateCallback]()
		                                                             {
			                                                             // Resolve the loaded UClass from the soft reference.
			                                                             UClass* LoadedWidgetClass = InSoftWidgetClass.
					                                                             Get();

			                                                             check(LoadedWidgetClass &&
				                                                             CreatedPrimaryLayout);

			                                                             // Resolve the target stack by tag from the primary layout.
			                                                             UCommonActivatableWidgetContainerBase*
					                                                             FoundWidgetStack = CreatedPrimaryLayout
					                                                             ->FindWidgetStackByTag(
						                                                             InWidgetStackTag);
			                                                             check(FoundWidgetStack);

			                                                             // Create the widget inside the stack.
			                                                             // AddWidget's lambda fires OnCreatedBeforePush before the widget is pushed,
			                                                             // giving the caller a chance to initialize it (e.g., InitConfirmScreen).
			                                                             UWidget_ActivatableBase* CreatedWidget =
					                                                             FoundWidgetStack->AddWidget<
						                                                             UWidget_ActivatableBase>(
						                                                             LoadedWidgetClass,
						                                                             [InAsyncPushStateCallback](
					                                                             UWidget_ActivatableBase&
					                                                             CreatedWidgetInstance)
						                                                             {
							                                                             InAsyncPushStateCallback(
								                                                             EAsyncPushWidgetState::OnCreatedBeforePush,
								                                                             &CreatedWidgetInstance);
						                                                             });

			                                                             // Fire AfterPush once the widget is active and visible in the stack.
			                                                             InAsyncPushStateCallback(
				                                                             EAsyncPushWidgetState::AfterPush,
				                                                             CreatedWidget);
		                                                             }));
}

void UFrontendUISubsystem::PushConfirmScreenToModalStackAsync(EConfirmScreenType InScreenType,
                                                              const FText& InScreenTitle,
                                                              const FText& InScreenMsg,
                                                              TFunction<void(EConfirmScreenButtonType)>
                                                              InButtonClickedCallback)
{
	// Build the appropriate info object based on the requested screen type.
	// The info object carries the title, message, and button configuration
	// that UWidget_ConfirmScreen uses to populate its dynamic entry box.
	UConfirmScreenInfoObject* CreatedInfoObject = nullptr;

	switch (InScreenType)
	{
	case EConfirmScreenType::Ok:
		CreatedInfoObject = UConfirmScreenInfoObject::CreateOKScreen(InScreenTitle,
		                                                             InScreenMsg);
		break;

	case EConfirmScreenType::YesNo:
		CreatedInfoObject = UConfirmScreenInfoObject::CreateYesNoScreen(InScreenTitle,
		                                                                InScreenMsg);
		break;

	case EConfirmScreenType::OKCancel:
		CreatedInfoObject = UConfirmScreenInfoObject::CreateOkCancelScreen(InScreenTitle,
		                                                                   InScreenMsg);
		break;

	case EConfirmScreenType::Unknown:
		// Unknown is a sentinel value — should never be passed intentionally.
		UE_LOG(LogFrontendUISubsystem,
		       Warning,
		       TEXT("PushConfirmScreenToModalStackAsync called with EConfirmScreenType::Unknown. Ignoring request."));
		return;

	default: UE_LOG(LogFrontendUISubsystem,
	                Warning,
	                TEXT(
		                "PushConfirmScreenToModalStackAsync called with unhandled EConfirmScreenType. Ignoring request."
	                ));
		return;
	}

	check(CreatedInfoObject);

	// Push the confirm screen widget asynchronously onto the modal stack.
	// The OnCreatedBeforePush phase initializes the screen before it becomes visible.
	PushSoftWidgetToStackAsync(FrontendUIGameplayTags::FrontendUI_WidgetStack_Modal,
	                           UFrontendUIFunctionLibrary::GetWidgetClassByTag(
		                           FrontendUIGameplayTags::FrontendUI_Widget_ConfirmScreen),
	                           [CreatedInfoObject, InButtonClickedCallback](EAsyncPushWidgetState InPushState,
	                                                                        UWidget_ActivatableBase* PushedWidget)
	                           {
		                           if (InPushState == EAsyncPushWidgetState::OnCreatedBeforePush)
		                           {
			                           // Initialize the confirm screen with its data before it becomes visible.
			                           // CastChecked here is safe: the tag guarantees this is always a UWidget_ConfirmScreen.
			                           UWidget_ConfirmScreen* CreatedConfirmScreen = CastChecked<UWidget_ConfirmScreen>(
				                           PushedWidget);
			                           CreatedConfirmScreen->InitConfirmScreen(CreatedInfoObject,
			                                                                   InButtonClickedCallback);
		                           }
	                           });
}
