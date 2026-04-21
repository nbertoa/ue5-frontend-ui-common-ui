#include "Widgets/Components/FrontendUICommonButtonBase.h"

#include "CommonTextBlock.h"
#include "Subsystems/FrontendUISubsystem.h"

void UFrontendUICommonButtonBase::SetButtonText(FText InText)
{
	// Skip silently if the text block is not bound or the text is empty.
	// BindWidgetOptional means absence of the text block is a valid configuration.
	if (CommonTextBlock_ButtonText && !InText.IsEmpty())
	{
		CommonTextBlock_ButtonText->SetText(bUserUpperCaseForButtonText
			                                    ? InText.ToUpper()
			                                    : InText);
	}
}

FText UFrontendUICommonButtonBase::GetButtonDisplayText() const
{
	if (CommonTextBlock_ButtonText)
	{
		return CommonTextBlock_ButtonText->GetText();
	}

	// Return empty text if no text block is bound — callers must handle this case.
	return FText::GetEmpty();
}

void UFrontendUICommonButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Apply the default display text at design time so the editor preview reflects
	// the configured text without needing to run the game.
	SetButtonText(ButtonDisplayText);
}

void UFrontendUICommonButtonBase::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();

	// Propagate the updated style class to the text block.
	// Common UI calls this automatically when the button state changes
	// (normal → hovered → pressed → disabled), keeping text style in sync.
	if (CommonTextBlock_ButtonText && GetCurrentTextStyleClass())
	{
		CommonTextBlock_ButtonText->SetStyle(GetCurrentTextStyleClass());
	}
}

void UFrontendUICommonButtonBase::NativeOnHovered()
{
	Super::NativeOnHovered();

	// Only broadcast if this button has a non-empty description text.
	// Buttons without descriptions (ButtonDescriptionText is empty) skip the broadcast
	// to avoid clearing a valid description shown by a previously hovered button.
	if (!ButtonDescriptionText.IsEmpty())
	{
		UFrontendUISubsystem* FrontendUISubsystem = UFrontendUISubsystem::Get(this);

		if (!ensureMsgf(FrontendUISubsystem,
		                TEXT("UFrontendUICommonButtonBase::NativeOnHovered — FrontendUISubsystem is null.")))
		{
			return;
		}

		FrontendUISubsystem->OnButtonDescriptionTextUpdated.Broadcast(this,
		                                                              ButtonDescriptionText);
	}
}

void UFrontendUICommonButtonBase::NativeOnUnhovered()
{
	Super::NativeOnUnhovered();

	// Always broadcast an empty text on unhover to clear the description display,
	// regardless of whether this button had a description.
	UFrontendUISubsystem* FrontendUISubsystem = UFrontendUISubsystem::Get(this);

	if (!ensureMsgf(FrontendUISubsystem,
	                TEXT("UFrontendUICommonButtonBase::NativeOnUnhovered — FrontendUISubsystem is null.")))
	{
		return;
	}

	FrontendUISubsystem->OnButtonDescriptionTextUpdated.Broadcast(this,
	                                                              FText::GetEmpty());
}
