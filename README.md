# Frontend UI with Common UI — Unreal Engine 5.6 C++

A focused C++ prototype built to study and implement a complete frontend UI system using Epic's **Common UI** plugin in Unreal Engine 5.6. The project implements a title screen with a full options screen (Video, Audio, Gameplay, Controls tabs), a reusable confirmation dialog system, and tag-based widget stack navigation — all driven by C++ with Blueprint subclassing for asset configuration.

Built following the [UE5 C++ Advanced Frontend UI Programming](https://www.udemy.com/course/ureal-engine-5-cpp-advanced-frontend-ui-programming/) course as a structural foundation, with my own C++ implementation throughout.

📺 [Gameplay Demo on YouTube](https://www.youtube.com/watch?v=YdxnxSYz0ss)  
📝 [Full technical breakdown on my blog](https://nbertoa.wordpress.com/2025/12/26/unreal-5-6-c-frontend-ui-with-common-ui/)

---

## Tech Stack

| | |
|---|---|
| **Engine** | Unreal Engine 5.6 |
| **Language** | C++ with Blueprint subclassing for asset configuration |
| **Plugins** | Common UI, Enhanced Input System |
| **Editor Plugins** | [Electronic Nodes](https://www.unrealengine.com/marketplace/en-US/product/electronic-nodes), [Blueprint Assist](https://www.unrealengine.com/marketplace/en-US/product/blueprint-assist) |

---

## Architecture Overview

### Widget Stack System

The UI is organized around a single `UWidget_PrimaryLayout` root widget that owns named `UCommonActivatableWidgetContainerBase` stacks, each identified by a Native Gameplay Tag:

```
UWidget_PrimaryLayout
├── FrontendUI.WidgetStack.Frontend   → Title screen flow
├── FrontendUI.WidgetStack.GameHud    → In-game HUD
├── FrontendUI.WidgetStack.GameMenu   → Pause menus
└── FrontendUI.WidgetStack.Modal      → Blocking dialogs (confirmation screens)
```

Stacks are registered at runtime via `RegisterWidgetStack` and resolved by tag via `FindWidgetStackByTag`. No code holds direct references to stacks — everything goes through the tag registry.

### Async Widget Push Flow

All widget navigation is asynchronous. `UFrontendUISubsystem::PushSoftWidgetToStackAsync` async-loads the widget class via `AssetManager::StreamableManager`, then pushes it via a two-phase callback:

```
1. OnCreatedBeforePush — widget exists, not yet visible. Configure it here.
2. AfterPush           — widget is active in the stack. Set focus here.
```

Both phases are exposed as Blueprint output pins via `UAsyncAction_PushSoftWidget` and `UAsyncAction_PushConfirmScreen`.

### Tag-Based Asset Registry

`UFrontendUIDeveloperSettings` (Project Settings → Frontend UI Settings) maps Gameplay Tags to soft asset references:

- `FrontendUI.Widget.*` → `TSoftClassPtr<UWidget_ActivatableBase>` (widget classes)
- `FrontendUI.Image.*`  → `TSoftObjectPtr<UTexture2D>` (options screen images)

`UFrontendUIFunctionLibrary` provides the runtime lookup functions. Adding a new screen requires one new tag and one new entry in Project Settings — no C++ changes needed.

---

## Key Systems

### UFrontendUISubsystem

Central coordinator living on the `GameInstance`. Manages the primary layout widget reference and owns the widget push API. Not created on Dedicated Servers. Supports derived classes — `ShouldCreateSubsystem` skips the base if a derived class exists.

### Options Screen Architecture

The options screen uses a three-panel layout driven by a data object hierarchy:

```
UOptionsDataRegistry (factory + registry)
└── UListDataObject_Collection  (tab: Gameplay / Audio / Video / Controls)
    ├── UListDataObject_Collection   (sub-category: Volume / Graphics / etc.)
    │   ├── UListDataObject_Scalar       → slider entry
    │   ├── UListDataObject_String       → rotator entry
    │   ├── UListDataObject_StringBool   → ON/OFF toggle
    │   ├── UListDataObject_StringEnum   → enum selector
    │   ├── UListDataObject_StringInteger → integer preset selector
    │   └── UListDataObject_StringResolution → platform resolution list
    └── ...
```

**Model-View separation:** data objects own business logic and values; entry widgets own visual representation. They communicate via `OnListDataModified` multicast delegates — no polling.

### Dynamic Getter/Setter Binding

`FOptionsDataInteractionHelper` bridges data objects to `UFrontendUIGameUserSettings` properties at runtime using `FCachedPropertyPath` and `PropertyPathHelpers`. Each setting is bound with a single UFUNCTION path string:

```cpp
// Validated at compile time via GET_FUNCTION_NAME_STRING_CHECKED
#define MAKE_OPTIONS_DATA_CONTROL(FuncName) \
    MakeShared<FOptionsDataInteractionHelper>( \
        GET_FUNCTION_NAME_STRING_CHECKED(UFrontendUIGameUserSettings, FuncName))

OverallVolume->SetDataDynamicGetter(MAKE_OPTIONS_DATA_CONTROL(GetOverallVolume));
OverallVolume->SetDataDynamicSetter(MAKE_OPTIONS_DATA_CONTROL(SetOverallVolume));
```

`GET_FUNCTION_NAME_STRING_CHECKED` validates the function exists at compile time, preventing silent runtime failures from typos.

### Edit Conditions and Dependencies

Settings can declare edit conditions that lock them based on other settings:

- **V-Sync** is locked when Window Mode ≠ Fullscreen (forced to `false`).
- **Screen Resolution** is locked when Window Mode = Borderless Window (forced to max resolution).
- **Window Mode and Resolution** are locked in editor builds (packaged build only).

Dependencies are bidirectional: changing `OverallQuality` updates all individual quality settings; changing any individual quality setting notifies `OverallQuality` to re-read its scalability level (which may become "Custom").

### Confirmation Screen System

`UFrontendUISubsystem::PushConfirmScreenToModalStackAsync` pushes a typed confirmation dialog onto the modal stack. Three layouts are supported: `Ok`, `YesNo`, `OKCancel`. Configuration is carried by `UConfirmScreenInfoObject` (a data transfer object), keeping the screen widget decoupled from the system that requested it.

Buttons are generated dynamically via `UDynamicEntryBox`. Focus defaults to the last button (the "negative" option) to prevent accidental confirmation of destructive actions via gamepad/keyboard.

### Hardware Benchmark

`AFrontendUIPlayerController::OnPossess` automatically runs `UGameUserSettings::RunHardwareBenchmark` on first launch (when no previous benchmark results exist) and applies the results as initial video quality settings.

### UFrontendUIGameUserSettings

Custom `UGameUserSettings` extension adding game-specific settings: difficulty, volume levels (overall/music/SFX), audio flags (background audio, HDR audio), and display gamma. All properties are `UPROPERTY(Config)` — persisted automatically to `GameUserSettings.ini`. Display gamma is applied directly to `GEngine->DisplayGamma` for immediate effect.

---

## Project Structure

```
FrontendUI/
├── Config/
│   ├── DefaultGame.ini
│   ├── DefaultGameplayTags.ini
│   └── DefaultInput.ini
├── Source/
│   └── FrontendUI/
│       ├── AsyncActions/
│       │   ├── AsyncAction_PushConfirmScreen   # Async BP node for confirmation dialogs
│       │   └── AsyncAction_PushSoftWidget      # Async BP node for widget push
│       ├── Controllers/
│       │   └── FrontendUIPlayerController      # Camera setup + hardware benchmark
│       ├── FrontendUIFunctionLibrary           # Tag-based asset lookup functions
│       ├── FrontendUIGameplayTags              # Native Gameplay Tag declarations
│       ├── FrontendUISettings/
│       │   ├── FrontendUIDeveloperSettings     # Project Settings tag→asset registry
│       │   └── FrontendUIGameUserSettings      # Custom GameUserSettings extension
│       ├── FrontendUITypes/
│       │   ├── FrontendUIEnumTypes             # EConfirmScreenType, EOptionsListDataModifyReason
│       │   └── FrontendUIStructTypes           # FOptionsDataEditConditionDescriptor
│       ├── Subsystems/
│       │   └── FrontendUISubsystem             # Central coordinator (GameInstance subsystem)
│       └── Widgets/
│           ├── Components/
│           │   ├── FrontendUICommonButtonBase  # Button with description broadcasting
│           │   ├── FrontendUICommonListView    # Data-driven entry widget generation
│           │   ├── FrontendUICommonRotator     # Text-based option selection
│           │   └── FrontendUITabListWidgetBase # Simplified tab registration API
│           ├── Options/
│           │   ├── DataAsset_DataListEntryMapping  # Data object → widget class mapping
│           │   ├── DataObjects/
│           │   │   ├── ListDataObject_Base         # Abstract base with edit conditions
│           │   │   ├── ListDataObject_Collection   # Tab/category grouping node
│           │   │   ├── ListDataObject_Value        # Base for getter/setter-bound data
│           │   │   ├── ListDataObject_Scalar       # Float slider with display/output ranges
│           │   │   ├── ListDataObject_String       # Rotator-based string options
│           │   │   ├── ListDataObject_StringResolution # Platform resolution list
│           │   │   └── (StringBool, StringEnum, StringInteger — in ListDataObject_String)
│           │   ├── ListEntries/
│           │   │   ├── Widget_ListEntry_Base       # Abstract entry with gamepad focus routing
│           │   │   ├── Widget_ListEntry_Scalar     # Slider entry widget
│           │   │   └── Widget_ListEntry_String     # Rotator + prev/next buttons entry
│           │   ├── OptionsDataInteractionHelper    # FCachedPropertyPath getter/setter bridge
│           │   ├── OptionsDataRegistry             # Factory + registry for all options data
│           │   ├── Widget_OptionsDetailsView       # Contextual info panel
│           │   └── Widget_OptionsScreen            # Main options screen coordinator
│           ├── Widget_ActivatableBase          # Base for all activatable screens
│           ├── Widget_ConfirmScreen            # Dynamic button confirmation dialog
│           └── Widget_PrimaryLayout            # Root layout with named stack registry
└── FrontendUI.uproject
```

---

## About

**Nicolás Bertoa** — Unreal Engine developer with 5+ years in UE5 C++, previously 9+ years R&D at DreamWorks and 5+ years R&D at Sony. Focused on C++, gameplay systems, and 3D math.

🌐 [Portfolio](https://nbertoa.wordpress.com) | 🎬 [Demo Reels](https://nbertoa.wordpress.com/demo-reels/)
[FrontendUI_README.md](https://github.com/user-attachments/files/26946527/FrontendUI_README.md)
