#include "Widgets/Options/Widget_OptionsDetailsView.h"

#include "CommonTextBlock.h"
#include "CommonLazyImage.h"
#include "CommonRichTextBlock.h"
#include "Widgets/Options/DataObjects/ListDataObject_Base.h"

void UWidget_OptionsDetailsView::UpdateDetailsViewInfo(UListDataObject_Base* InDataObject,
                                                       const FString& InEntryWidgetClassName) const
{
	// Soft fail: null data object is a valid transient state during tab switches.
	if (!InDataObject)
	{
		return;
	}

	// Update title.
	CommonTextBlock_Title->SetText(InDataObject->GetDataDisplayName());

	// Lazily load and display the description image if one is assigned.
	// UCommonLazyImage loads the texture asynchronously, avoiding game thread stalls.
	// Collapse the image widget entirely when no image is assigned to avoid
	// showing an empty placeholder in the UI layout.
	if (!InDataObject->GetSoftDescriptionImage().IsNull())
	{
		CommonLazyImage_DescriptionImage->SetBrushFromLazyTexture(InDataObject->GetSoftDescriptionImage());
		CommonLazyImage_DescriptionImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		CommonLazyImage_DescriptionImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Update rich-text description.
	CommonRichText_Description->SetText(InDataObject->GetDescriptionRichText());

	// Populate dynamic debug details with the data object and entry widget class names.
	// This helps verify correct type mapping during development without needing the debugger.
	const FString DynamicDetails = FString::Printf(
		TEXT("Data Object Class: <Bold>%s</>\n\nEntry Widget Class: <Bold>%s</>"),
		*InDataObject->GetClass()->GetName(),
		*InEntryWidgetClassName);
	CommonRichText_DynamicDetails->SetText(FText::FromString(DynamicDetails));

	// Show the disabled reason only when the setting is not editable.
	// IsDataCurrentlyEditable also evaluates forced value conditions as a side effect,
	// so this call may modify the data object's current value.
	CommonRichText_DisabledReason->SetText(InDataObject->IsDataCurrentlyEditable()
		                                       ? FText::GetEmpty()
		                                       : InDataObject->GetDisabledRichText());
}

void UWidget_OptionsDetailsView::ClearDetailsViewInfo()
{
	CommonTextBlock_Title->SetText(FText::GetEmpty());
	CommonLazyImage_DescriptionImage->SetVisibility(ESlateVisibility::Collapsed);
	CommonRichText_Description->SetText(FText::GetEmpty());
	CommonRichText_DynamicDetails->SetText(FText::GetEmpty());
	CommonRichText_DisabledReason->SetText(FText::GetEmpty());
}

void UWidget_OptionsDetailsView::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Clear on initialization to ensure no stale data is displayed
	// if the widget is constructed before any selection is made.
	ClearDetailsViewInfo();
}
