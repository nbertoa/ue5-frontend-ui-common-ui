#include "Widgets/Components/FrontendUICommonRotator.h"

#include "CommonTextBlock.h"

void UFrontendUICommonRotator::SetSelectedOptionByText(const FText& InTextOption)
{
	// Search the rotator's label array for an exact text match.
	// IndexOfByPredicate returns INDEX_NONE if no match is found.
	const int32 FoundIndex = TextLabels.IndexOfByPredicate([InTextOption](const FText& TextItem) -> bool
	{
		return TextItem.EqualTo(InTextOption);
	});

	if (FoundIndex != INDEX_NONE)
	{
		// Match found — select the corresponding index via the standard rotator API.
		// This triggers OnRotatedEvent, keeping all listeners in sync.
		SetSelectedItem(FoundIndex);
	}
	else
	{
		// Fallback: the text is not in the label array (e.g., a custom resolution
		// or an out-of-range value). Set the text directly on the display text block
		// to preserve visual accuracy without crashing or asserting.
		// Note: this does NOT update the selected index — the rotator remains at
		// its previous index internally, but displays the custom text visually.
		MyText->SetText(InTextOption);
	}
}
