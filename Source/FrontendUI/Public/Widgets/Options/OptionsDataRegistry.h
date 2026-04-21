#pragma once

#include "CoreMinimal.h"
#include "OptionsDataRegistry.generated.h"

/**
 * @class UOptionsDataRegistry
 * @brief Builds and owns the complete options data object hierarchy for the options screen.
 *
 * Creates and configures all UListDataObject_Collection and child UListDataObject_Base
 * instances that represent the options screen's settings. Each tab (Gameplay, Audio,
 * Video, Controls) is initialized by a dedicated private method.
 *
 * @section Architecture Factory + Registry Pattern
 * UOptionsDataRegistry acts as both a factory (creating data objects) and a registry
 * (storing and providing access to the created hierarchy). It is owned by UWidget_OptionsScreen
 * as a Transient UPROPERTY, giving it the same lifetime as the screen widget.
 *
 * Data objects are configured with:
 * - Display metadata (name, description, image).
 * - Dynamic getter/setter helpers (FOptionsDataInteractionHelper) bound to
 *   UFrontendUIGameUserSettings UFUNCTION paths.
 * - Default values for reset support.
 * - Edit conditions that determine editability based on other settings.
 * - Dependency relationships that trigger re-evaluation when a dependency changes.
 *
 * @section Tab Initialization Order
 * Tabs are initialized in declaration order: Gameplay → Audio → Video → Controls.
 * This order determines the tab display order in UFrontendUITabListWidgetBase.
 *
 * @section List Source Items
 * GetListSourceItemsBySelectedTabID recursively collects all leaf data objects
 * (non-collection children) under the selected tab's collection, building the
 * flat list displayed in UFrontendUICommonListView.
 *
 * @see UWidget_OptionsScreen for where this registry is created and used.
 * @see FOptionsDataInteractionHelper for the dynamic getter/setter binding.
 * @see UListDataObject_Base and subclasses for the data object types used here.
 */
UCLASS()
class FRONTENDUI_API UOptionsDataRegistry : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief Initializes the registry by building all tab collections and their child data objects.
	 *
	 * Must be called once immediately after the registry is created (NewObject).
	 * Calls InitGameplayCollectionTab, InitAudioCollectionTab, InitVideoCollectionTab,
	 * and InitControlCollectionTab in order.
	 *
	 * @param InOwningLocalPlayer The local player used to resolve player-specific settings context.
	 */
	void InitOptionsDataRegistry(ULocalPlayer* InOwningLocalPlayer);

	/**
	 * @brief Returns all registered tab collections.
	 * Used by UWidget_OptionsScreen::NativeOnActivated to register tabs in the tab list widget.
	 *
	 * @return Const reference to the ordered array of tab collection data objects.
	 */
	const TArray<class UListDataObject_Collection*>& GetRegisteredOptionsTabCollections() const
	{
		return RegisteredOptionsTabCollections;
	}

	/**
	 * @brief Returns the flat list of data objects to display for the selected tab.
	 *
	 * Finds the collection with a matching DataID, then recursively collects all
	 * leaf data objects (non-collection children) under it. Collections within the
	 * hierarchy are skipped — only leaf nodes are displayed in the list view.
	 *
	 * @param InSelectedTabID The DataID of the selected tab collection (set in the init methods).
	 * @return Flat array of UListDataObject_Base pointers for the list view source items.
	 */
	TArray<class UListDataObject_Base*> GetListSourceItemsBySelectedTabID(const FName& InSelectedTabID) const;

private:
	/**
	 * @brief Recursively collects all leaf (non-collection) data objects under InParentData.
	 *
	 * Used by GetListSourceItemsBySelectedTabID to flatten the data hierarchy
	 * into a linear array suitable for UFrontendUICommonListView.
	 *
	 * @param InParentData      The parent data object to search under.
	 * @param OutFoundChildListData Output array of collected leaf data objects.
	 */
	static void FindChildListDataRecursively(UListDataObject_Base* InParentData,
	                                         TArray<UListDataObject_Base*>& OutFoundChildListData);

	/**
	 * @brief Builds the Gameplay tab collection with difficulty and related settings.
	 * Creates a UListDataObject_Collection with DataID "Gameplay" and populates
	 * it with difficulty selection data objects.
	 */
	void InitGameplayCollectionTab();

	/**
	 * @brief Builds the Audio tab collection with volume and audio mode settings.
	 * Creates a UListDataObject_Collection with DataID "Audio" and populates it with:
	 * - Overall, Music, and SFX volume sliders (UListDataObject_Scalar).
	 * - Allow Background Audio toggle (UListDataObject_StringBool).
	 * - HDR Audio Mode toggle (UListDataObject_StringBool).
	 */
	void InitAudioCollectionTab();

	/**
	 * @brief Builds the Video tab collection with display and quality settings.
	 * Creates a UListDataObject_Collection with DataID "Video" and populates it with:
	 * - Resolution selector (UListDataObject_StringResolution).
	 * - Window mode selector (UListDataObject_StringEnum).
	 * - Display gamma slider (UListDataObject_Scalar).
	 * Configures resolution dependency on window mode (resolution locked when not fullscreen).
	 */
	void InitVideoCollectionTab();

	/**
	 * @brief Builds the Controls tab collection with input-related settings.
	 * Creates a UListDataObject_Collection with DataID "Controls".
	 * Extended in derived registries for project-specific control remapping.
	 */
	void InitControlCollectionTab();

	/**
	 * @brief Ordered array of tab collection data objects registered during initialization.
	 * Order determines tab display order in UFrontendUITabListWidgetBase.
	 * Transient: rebuilt each session when the options screen is first opened.
	 */
	UPROPERTY(Transient)
	TArray<class UListDataObject_Collection*> RegisteredOptionsTabCollections;
};
