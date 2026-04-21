#include "Widgets/Widget_ConfirmScreen.h"

#include "CommonTextBlock.h"
#include "Components/DynamicEntryBox.h"
#include "ICommonInputModule.h"
#include "Widgets/Components/FrontendUICommonButtonBase.h"

UConfirmScreenInfoObject* UConfirmScreenInfoObject::CreateOKScreen(const FText& InScreenTitle,
                                                                   const FText& InScreenMsg)
{
	UConfirmScreenInfoObject* InfoObject = NewObject<UConfirmScreenInfoObject>();
	InfoObject->ScreenTitle = InScreenTitle;
	InfoObject->ScreenMessage = InScreenMsg;

	// "Ok" maps to Closed rather than Confirmed — the user is acknowledging
	// information, not confirming a binary choice.
	FConfirmScreenButtonInfo OKButtonInfo;
	OKButtonInfo.ConfirmScreenButtonType = EConfirmScreenButtonType::Closed;
	OKButtonInfo.ButtonTextToDisplay = FText::FromString(TEXT("Ok"));

	InfoObject->AvailableScreenButtons.Add(OKButtonInfo);

	return InfoObject;
}

UConfirmScreenInfoObject* UConfirmScreenInfoObject::CreateYesNoScreen(const FText& InScreenTitle,
                                                                      const FText& InScreenMsg)
{
	UConfirmScreenInfoObject* InfoObject = NewObject<UConfirmScreenInfoObject>();
	InfoObject->ScreenTitle = InScreenTitle;
	InfoObject->ScreenMessage = InScreenMsg;

	FConfirmScreenButtonInfo YesButtonInfo;
	YesButtonInfo.ConfirmScreenButtonType = EConfirmScreenButtonType::Confirmed;
	YesButtonInfo.ButtonTextToDisplay = FText::FromString(TEXT("Yes"));

	FConfirmScreenButtonInfo NoButtonInfo;
	NoButtonInfo.ConfirmScreenButtonType = EConfirmScreenButtonType::Cancelled;
	NoButtonInfo.ButtonTextToDisplay = FText::FromString(TEXT("No"));

	// "Yes" is added first, "No" second.
	// NativeGetDesiredFocusTarget returns the last entry ("No"),
	// ensuring gamepad/keyboard focus defaults to the safe option.
	InfoObject->AvailableScreenButtons.Add(YesButtonInfo);
	InfoObject->AvailableScreenButtons.Add(NoButtonInfo);

	return InfoObject;
}

UConfirmScreenInfoObject* UConfirmScreenInfoObject::CreateOkCancelScreen(const FText& InScreenTitle,
                                                                         const FText& InScreenMsg)
{
	UConfirmScreenInfoObject* InfoObject = NewObject<UConfirmScreenInfoObject>();
	InfoObject->ScreenTitle = InScreenTitle;
	InfoObject->ScreenMessage = InScreenMsg;

	FConfirmScreenButtonInfo OkButtonInfo;
	OkButtonInfo.ConfirmScreenButtonType = EConfirmScreenButtonType::Confirmed;
	OkButtonInfo.ButtonTextToDisplay = FText::FromString(TEXT("Ok"));

	FConfirmScreenButtonInfo CancelButtonInfo;
	CancelButtonInfo.ConfirmScreenButtonType = EConfirmScreenButtonType::Cancelled;
	CancelButtonInfo.ButtonTextToDisplay = FText::FromString(TEXT("Cancel"));

	// "Ok" first, "Cancel" last — focus defaults to "Cancel" for safety.
	InfoObject->AvailableScreenButtons.Add(OkButtonInfo);
	InfoObject->AvailableScreenButtons.Add(CancelButtonInfo);

	return InfoObject;
}

void UWidget_ConfirmScreen::InitConfirmScreen(UConfirmScreenInfoObject* InScreenInfoObject,
                                              TFunction<void(EConfirmScreenButtonType)> InClickedButtonCallback)
{
	check(InScreenInfoObject && CommonTextBlock_Title && CommonTextBlock_Message && DynamicEntryBox_Buttons);

	CommonTextBlock_Title->SetText(InScreenInfoObject->ScreenTitle);
	CommonTextBlock_Message->SetText(InScreenInfoObject->ScreenMessage);

	// Clear existing buttons if this screen is being reused.
	// OnClicked delegates are cleared per entry to prevent stale callbacks
	// from a previous push from firing on the next push.
	if (DynamicEntryBox_Buttons->GetNumEntries() != 0)
	{
		DynamicEntryBox_Buttons->Reset<UFrontendUICommonButtonBase>(
			[](const UFrontendUICommonButtonBase& ExistingButton)
			{
				ExistingButton.OnClicked().Clear();
			});
	}

	check(!InScreenInfoObject->AvailableScreenButtons.IsEmpty());

	for (const FConfirmScreenButtonInfo& AvailableButtonInfo : InScreenInfoObject->AvailableScreenButtons)
	{
		FDataTableRowHandle InputActionRowHandle;

		// Assign the default back input action to buttons that close/cancel the screen.
		// This ensures the back button (gamepad B, keyboard Escape) triggers these buttons
		// correctly without requiring per-button input action configuration.
		switch (AvailableButtonInfo.ConfirmScreenButtonType)
		{
		case EConfirmScreenButtonType::Cancelled:
		case EConfirmScreenButtonType::Closed:
			InputActionRowHandle = ICommonInputModule::GetSettings().GetDefaultBackAction();
			break;

		default:
			// Confirmed buttons have no default input action — they require explicit click/select.
			break;
		}

		UFrontendUICommonButtonBase* AddedButton = DynamicEntryBox_Buttons->CreateEntry<UFrontendUICommonButtonBase>();
		AddedButton->SetButtonText(AvailableButtonInfo.ButtonTextToDisplay);
		AddedButton->SetTriggeringInputAction(InputActionRowHandle);

		// Capture button type by value — safe because AvailableButtonInfo is copied
		// into the lambda scope, not referenced by pointer.
		AddedButton->OnClicked().AddLambda([InClickedButtonCallback, AvailableButtonInfo, this]()
		{
			InClickedButtonCallback(AvailableButtonInfo.ConfirmScreenButtonType);

			// Deactivate the widget after the callback — removes it from the modal stack
			// and returns focus to the widget beneath it.
			DeactivateWidget();
		});
	}

	// Set initial focus to the last button (the "safe" / negative option).
	// This prevents accidental confirmation of destructive actions via gamepad/keyboard.
	if (DynamicEntryBox_Buttons->GetNumEntries() != 0)
	{
		DynamicEntryBox_Buttons->GetAllEntries().Last()->SetFocus();
	}
}

UWidget* UWidget_ConfirmScreen::NativeGetDesiredFocusTarget() const
{
	// Return the last button as the desired focus target.
	// For YesNo and OkCancel screens, this is the negative option (No/Cancel),
	// preventing accidental confirmation via gamepad or keyboard navigation.
	if (DynamicEntryBox_Buttons->GetNumEntries() != 0)
	{
		return DynamicEntryBox_Buttons->GetAllEntries().Last();
	}

	return Super::NativeGetDesiredFocusTarget();
}
