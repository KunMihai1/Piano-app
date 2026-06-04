# Play Screen UI Improvements Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restyle all play-screen controls to use the dark AppColours palette with a shared `PlayScreenLookAndFeel`, and fix the blank loading label when switching styles with the internal SFZ synth.

**Architecture:** A new `PlayScreenLookAndFeel` class (standalone header, no source file needed) is shared across `MainComponent`, `SectionGroupComponent`, `TrackListComponent`, and `StylesListComponent`. Each class holds a LAF member declared before its button members so destruction order is safe. All colour changes use `AppColours` constants already defined in `AppColours.h`.

**Tech Stack:** JUCE 7, C++17, Windows. Build via the `.jucer` project in Visual Studio. No test runner — verify by building and running the app.

---

## File Map

| Action | File | Responsibility |
|---|---|---|
| **Create** | `Source/PlayScreenLookAndFeel.h` | Shared LookAndFeel: rounded buttons, role-coded borders, toggle styling |
| **Modify** | `Source/MainComponent.h` | Add `#include` + `PlayScreenLookAndFeel playScreenLAF` member |
| **Modify** | `Source/MainComponent.cpp` | Header paint, button inits, toggle inits, loading label |
| **Modify** | `Source/SectionGroupComponent.h` | Add LAF member before existing members |
| **Modify** | `Source/SectionGroupComponent.cpp` | Apply LAF to all buttons, style title label |
| **Modify** | `Source/SectionsComponent.cpp` | Active button colour → teal |
| **Modify** | `Source/displayGUI.h` | Add LAF members to TrackListComponent, StylesListComponent |
| **Modify** | `Source/displayGUI.cpp` | Apply LAF + role colours to all display buttons, dark list rows, StylesListComponent paint |

---

## Task 1: Create PlayScreenLookAndFeel.h

**Files:**
- Create: `Source/PlayScreenLookAndFeel.h`

- [ ] **Step 1: Create the file with the full class**

`Source/PlayScreenLookAndFeel.h`:
```cpp
#pragma once
#include <JuceHeader.h>
#include "AppColours.h"

class PlayScreenLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PlayScreenLookAndFeel()
    {
        setColour(juce::TextButton::textColourOffId,      AppColours::titleText);
        setColour(juce::TextButton::textColourOnId,       AppColours::titleText);
        setColour(juce::TextButton::buttonColourId,       AppColours::accent2);
        setColour(juce::TextButton::buttonOnColourId,     AppColours::accent3);
        setColour(juce::ToggleButton::textColourId,       AppColours::titleText);
        setColour(juce::ToggleButton::tickColourId,       AppColours::accent3);
        setColour(juce::ToggleButton::tickDisabledColourId, AppColours::rowLabel);
    }

    void drawButtonBackground(juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool isMouseOverButton,
                               bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        constexpr float cornerRadius = 6.0f;

        juce::Colour fill = AppColours::cardBg;
        if (isButtonDown)       fill = fill.darker(0.15f);
        else if (isMouseOverButton) fill = fill.brighter(0.15f);

        g.setColour(fill);
        g.fillRoundedRectangle(bounds, cornerRadius);

        g.setColour(backgroundColour.withAlpha(isMouseOverButton ? 1.0f : 0.75f));
        g.drawRoundedRectangle(bounds, cornerRadius, 1.5f);
    }

    juce::Font getTextButtonFont(juce::TextButton&, int) override
    {
        return juce::Font(13.0f);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayScreenLookAndFeel)
};
```

- [ ] **Step 2: Verify the file exists**

```
dir "Source\PlayScreenLookAndFeel.h"
```
Expected: file listed, no error.

- [ ] **Step 3: Commit**

```bash
git add Source/PlayScreenLookAndFeel.h
git commit -m "feat: add PlayScreenLookAndFeel for dark rounded button style"
```

---

## Task 2: Add LAF member to MainComponent

**Files:**
- Modify: `Source/MainComponent.h`

- [ ] **Step 1: Add include after the existing JUCE/project includes**

In `MainComponent.h`, the include block ends around the `#include "AudioHandler.h"` line. Add immediately after it:
```cpp
#include "PlayScreenLookAndFeel.h"
```

- [ ] **Step 2: Add the LAF member in the private section**

The private section currently has `CustomLookAndFeel customLookAndFeel;` around line 301. Add `playScreenLAF` immediately after it so it is declared (and thus destroyed) after all button members:

Change:
```cpp
    CustomLookAndFeel customLookAndFeel; ///< Custom look and feel
```
To:
```cpp
    CustomLookAndFeel customLookAndFeel; ///< Custom look and feel
    PlayScreenLookAndFeel playScreenLAF;
```

- [ ] **Step 3: Build to verify the header compiles**

Open the `.jucer` project in Visual Studio and build (Ctrl+Shift+B). Expected: no errors.

- [ ] **Step 4: Commit**

```bash
git add Source/MainComponent.h
git commit -m "feat: add playScreenLAF member to MainComponent"
```

---

## Task 3: Update HeaderPanel paint

**Files:**
- Modify: `Source/MainComponent.cpp` (function `MainComponent::headerPanel::paint`, at the bottom of the file)

- [ ] **Step 1: Replace the paint body**

Find and replace the entire `headerPanel::paint` function (currently a crimson-to-gold gradient):

Old code:
```cpp
void MainComponent::headerPanel::paint(juce::Graphics& g)
{
    juce::Colour startColour = juce::Colour(128, 0, 32);
    juce::Colour endColour = juce::Colour(212, 175, 55); 

    juce::ColourGradient gradient(startColour, 0, 0, endColour, 0, 50, false);
    g.setGradientFill(gradient);
    g.fillAll();
}
```

New code:
```cpp
void MainComponent::headerPanel::paint(juce::Graphics& g)
{
    // Base dark fill
    g.fillAll(AppColours::panelBg);

    // Subtle purple gradient overlay (fades from top)
    juce::ColourGradient overlay(AppColours::accent2.withAlpha(0.18f), 0.0f, 0.0f,
                                 juce::Colours::transparentBlack, 0.0f, (float)getHeight(), false);
    g.setGradientFill(overlay);
    g.fillAll();

    // Top accent strip
    g.setColour(AppColours::accent2);
    g.fillRect(0, 0, getWidth(), 3);

    // Bottom teal separator (horizon between controls and note area)
    g.setColour(AppColours::accent3);
    g.fillRect(0, getHeight() - 2, getWidth(), 2);
}
```

- [ ] **Step 2: Build and run the app, press Play, verify the header shows dark-indigo with purple top strip and teal bottom line**

- [ ] **Step 3: Commit**

```bash
git add Source/MainComponent.cpp
git commit -m "feat: restyle headerPanel to dark AppColours theme"
```

---

## Task 4: Apply PlayScreenLookAndFeel to header TextButtons

**Files:**
- Modify: `Source/MainComponent.cpp` (six init functions)

Apply `playScreenLAF` and set `buttonColourId` (used as border colour) in each init function. Do **not** remove existing `setMouseCursor` or `onClick` lines.

- [ ] **Step 1: Update `homeButtonInit`**

Add two lines after `homeButton.setButtonText("Home");`:
```cpp
    homeButton.setButtonText("Home");
    homeButton.setLookAndFeel(&playScreenLAF);
    homeButton.setColour(juce::TextButton::buttonColourId, AppColours::accent2);
    homeButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
```

- [ ] **Step 2: Update `colourSelectorButtonInit`**

Add two lines after `colourSelectorButton.setButtonText("Particle colours");`:
```cpp
    colourSelectorButton.setButtonText("Particle colours");
    colourSelectorButton.setLookAndFeel(&playScreenLAF);
    colourSelectorButton.setColour(juce::TextButton::buttonColourId, AppColours::accent2);
    colourSelectorButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
```

- [ ] **Step 3: Update `instrumentSelectorButtonInit`**

Add two lines after `instrumentSelectorButton.setButtonText("Instruments");`:
```cpp
    instrumentSelectorButton.setButtonText("Instruments");
    instrumentSelectorButton.setLookAndFeel(&playScreenLAF);
    instrumentSelectorButton.setColour(juce::TextButton::buttonColourId, AppColours::accent2);
    instrumentSelectorButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
```

- [ ] **Step 4: Update `chordHelperButtonInit`**

Add two lines after `chordHelperButton.setButtonText("Chord helper");`:
```cpp
    chordHelperButton.setButtonText("Chord helper");
    chordHelperButton.setLookAndFeel(&playScreenLAF);
    chordHelperButton.setColour(juce::TextButton::buttonColourId, AppColours::accent2);
    chordHelperButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
```

- [ ] **Step 5: Update `saveRecordingButtonInit`**

Add two lines after `saveRecordingButton.setButtonText("Save recording");`:
```cpp
    saveRecordingButton.setButtonText("Save recording");
    saveRecordingButton.setLookAndFeel(&playScreenLAF);
    saveRecordingButton.setColour(juce::TextButton::buttonColourId, AppColours::accent2);
    saveRecordingButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
```

- [ ] **Step 6: Update `playRecordingButtonInit`**

Add two lines after `playRecordingFileButton.setButtonText("Play file recording");`:
```cpp
    playRecordingFileButton.setButtonText("Play file recording");
    playRecordingFileButton.setLookAndFeel(&playScreenLAF);
    playRecordingFileButton.setColour(juce::TextButton::buttonColourId, AppColours::accent2);
    playRecordingFileButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
```

- [ ] **Step 7: Build and run — press Play and verify all six TextButtons show rounded dark-fill with purple border**

- [ ] **Step 8: Commit**

```bash
git add Source/MainComponent.cpp
git commit -m "feat: apply PlayScreenLookAndFeel to header TextButtons"
```

---

## Task 5: Update ToggleButton colours and loading label text

**Files:**
- Modify: `Source/MainComponent.cpp` (four init functions + constructor + `ensureAudioHandlerReady` + `displayInit`)

- [ ] **Step 1: Update `toggleButtonInit` — particle toggle colours**

Replace:
```cpp
    particleToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::green);
    particleToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);
```
With:
```cpp
    particleToggle.setColour(juce::ToggleButton::textColourId,        AppColours::titleText);
    particleToggle.setColour(juce::ToggleButton::tickColourId,         AppColours::accent3);
    particleToggle.setColour(juce::ToggleButton::tickDisabledColourId, AppColours::rowLabel);
```

- [ ] **Step 2: Update `toggleHandButtonsInit` — left hand toggle colours**

Replace:
```cpp
    leftHandInstrumentToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::green);
    leftHandInstrumentToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);
```
With:
```cpp
    leftHandInstrumentToggle.setColour(juce::ToggleButton::textColourId,        AppColours::titleText);
    leftHandInstrumentToggle.setColour(juce::ToggleButton::tickColourId,         AppColours::accent3);
    leftHandInstrumentToggle.setColour(juce::ToggleButton::tickDisabledColourId, AppColours::rowLabel);
```

- [ ] **Step 3: Update `toggleHandButtonsInit` — right hand toggle colours**

Replace:
```cpp
    rightHandInstrumentToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::green);
    rightHandInstrumentToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);
```
With:
```cpp
    rightHandInstrumentToggle.setColour(juce::ToggleButton::textColourId,        AppColours::titleText);
    rightHandInstrumentToggle.setColour(juce::ToggleButton::tickColourId,         AppColours::accent3);
    rightHandInstrumentToggle.setColour(juce::ToggleButton::tickDisabledColourId, AppColours::rowLabel);
```

- [ ] **Step 4: Update `annotationInit` — note annotation toggle colours**

Replace:
```cpp
    noteNumbersAnnotation.setColour(juce::ToggleButton::tickColourId, juce::Colours::green);
    noteNumbersAnnotation.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);
```
With:
```cpp
    noteNumbersAnnotation.setColour(juce::ToggleButton::textColourId,        AppColours::titleText);
    noteNumbersAnnotation.setColour(juce::ToggleButton::tickColourId,         AppColours::accent3);
    noteNumbersAnnotation.setColour(juce::ToggleButton::tickDisabledColourId, AppColours::rowLabel);
```

- [ ] **Step 5: Fix loading label initial text — in the `MainComponent` constructor**

Find the constructor block that sets up `openingAudioLabel` (around line 38):
```cpp
    openingAudioLabel.setText("Opening Audio Device...", juce::dontSendNotification);
```
Change to:
```cpp
    openingAudioLabel.setText("Preparing session...", juce::dontSendNotification);
```

- [ ] **Step 6: Fix `onSfzLoadStart` text — in `ensureAudioHandlerReady`**

Find the `onSfzLoadStart` lambda (around line 2174):
```cpp
        audioHandler->onSfzLoadStart = [this]() {
            openingAudioLabel.setText("Configuring instrument...", juce::dontSendNotification);
            openingAudioLabel.setVisible(true);
        };
```
Change to:
```cpp
        audioHandler->onSfzLoadStart = [this]() {
            openingAudioLabel.setText("Preparing style...", juce::dontSendNotification);
            openingAudioLabel.setVisible(true);
        };
```

- [ ] **Step 7: Show label during style switches — in `displayInit`**

Find the style-change lambda in `displayInit`. The relevant block currently reads:
```cpp
        if (MIDIDevice.isOpenAudioOUT())
            loadSfzForCurrentStyle(styleID);
```
Replace with:
```cpp
        if (MIDIDevice.isOpenAudioOUT())
        {
            openingAudioLabel.setText("Preparing style...", juce::dontSendNotification);
            openingAudioLabel.setVisible(true);
            loadSfzForCurrentStyle(styleID);
            // Fallback hide: if no SFZ file actually changed, onSfzLoadComplete won't fire
            juce::Timer::callAfterDelay(300,
                [safeThis = juce::Component::SafePointer<MainComponent>(this)]()
                {
                    if (safeThis)
                        safeThis->openingAudioLabel.setVisible(false);
                });
        }
```

- [ ] **Step 8: Build and run — switch styles with internal SFZ, verify "Preparing style..." appears**

- [ ] **Step 9: Commit**

```bash
git add Source/MainComponent.cpp
git commit -m "feat: update toggle colours + loading label text to Preparing style"
```

---

## Task 6: Style SectionGroupComponent buttons

**Files:**
- Modify: `Source/SectionGroupComponent.h`
- Modify: `Source/SectionGroupComponent.cpp`

- [ ] **Step 1: Add include + LAF member to the header**

In `SectionGroupComponent.h`, add after `#include "CustomToolTip.h"`:
```cpp
#include "PlayScreenLookAndFeel.h"
```

In the private section of `SectionGroupComponent`, add `PlayScreenLookAndFeel laf;` as the **first** private member (before `juce::Label titleLabel;`):

Old private section opening:
```cpp
private:
    juce::Label titleLabel;
    std::vector<std::unique_ptr<juce::TextButton>> buttons;
```
New:
```cpp
private:
    PlayScreenLookAndFeel laf;
    juce::Label titleLabel;
    std::vector<std::unique_ptr<juce::TextButton>> buttons;
```

- [ ] **Step 2: Style title label in `SectionGroupComponent` constructor**

In `SectionGroupComponent.cpp`, the constructor sets `titleLabel.setColour(juce::Label::textColourId, juce::Colours::white)`. Change to:
```cpp
    titleLabel.setColour(juce::Label::textColourId, AppColours::subtitleText);
```

- [ ] **Step 3: Apply LAF to each button created in the constructor loop**

Inside the `for (auto& name : buttonNames)` loop, after `addAndMakeVisible(*btn);` and before `buttons.push_back(...)`:
```cpp
        btn->setLookAndFeel(&laf);
        btn->setColour(juce::TextButton::buttonColourId, AppColours::accent2);
```

The loop after your change should look like:
```cpp
    for (auto& name : buttonNames)
    {
        auto btn = std::make_unique<juce::TextButton>(name);
        btn->setButtonText(name);
        btn->setMouseCursor(juce::MouseCursor::PointingHandCursor);
        activationMap[name] = false;

        btn->setLookAndFeel(&laf);
        btn->setColour(juce::TextButton::buttonColourId, AppColours::accent2);

        addAndMakeVisible(*btn);
        buttons.push_back(std::move(btn));
    }
```

- [ ] **Step 4: Apply LAF to the Break button (added for "variations" section)**

After the `if (title.equalsIgnoreCase("variations"))` block that creates the Break button, the button is pushed to `buttons`. Apply LAF right before `addAndMakeVisible(*btn)`:

Old:
```cpp
    if (title.equalsIgnoreCase("variations"))
    {
        auto btn = std::make_unique<juce::TextButton>("Break");
        btn->setButtonText("Break");
        btn->setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(*btn);
        buttons.push_back(std::move(btn));
    }
```
New:
```cpp
    if (title.equalsIgnoreCase("variations"))
    {
        auto btn = std::make_unique<juce::TextButton>("Break");
        btn->setButtonText("Break");
        btn->setMouseCursor(juce::MouseCursor::PointingHandCursor);
        btn->setLookAndFeel(&laf);
        btn->setColour(juce::TextButton::buttonColourId, AppColours::accent2);
        addAndMakeVisible(*btn);
        buttons.push_back(std::move(btn));
    }
```

- [ ] **Step 5: Build and run — press Play, verify Intros/Endings/Vars/Fills buttons show rounded dark style**

- [ ] **Step 6: Commit**

```bash
git add Source/SectionGroupComponent.h Source/SectionGroupComponent.cpp
git commit -m "feat: apply PlayScreenLookAndFeel to section group buttons"
```

---

## Task 7: Update SectionsComponent active button colour

**Files:**
- Modify: `Source/SectionsComponent.cpp`

- [ ] **Step 1: Change active colour from green to teal**

In `SectionsComponent.cpp`, find `applyChangeColour`:
```cpp
void StyleSectionComponent::applyChangeColour(juce::TextButton& button, bool activated)
{
    if (activated)
    {
        button.removeColour(juce::TextButton::buttonColourId);
    }
    else
    {
        button.setColour(juce::TextButton::buttonColourId, juce::Colours::green.withAlpha(0.8f));
    }

    button.repaint();
}
```
Replace with:
```cpp
void StyleSectionComponent::applyChangeColour(juce::TextButton& button, bool activated)
{
    if (activated)
    {
        button.setColour(juce::TextButton::buttonColourId, AppColours::accent2);
    }
    else
    {
        button.setColour(juce::TextButton::buttonColourId, AppColours::accent3.withAlpha(0.85f));
    }

    button.repaint();
}
```

Note: `activated` means the button **was** active (and is now being toggled off back to default purple). `!activated` (the `else` branch) is when it is being **activated** — fill with teal. `SectionsComponent.h` already includes `SectionGroupComponent.h` which now includes `PlayScreenLookAndFeel.h` which includes `AppColours.h`, so `AppColours` is available.

- [ ] **Step 2: Build and run — press an Intro/Fill button and verify it turns teal when active**

- [ ] **Step 3: Commit**

```bash
git add Source/SectionsComponent.cpp
git commit -m "feat: change section button active colour from green to teal"
```

---

## Task 8: Style Display and TrackList buttons

**Files:**
- Modify: `Source/displayGUI.h`
- Modify: `Source/displayGUI.cpp`

- [ ] **Step 1: Add include + LAF members to displayGUI.h**

In `displayGUI.h`, add after the last existing `#include`:
```cpp
#include "PlayScreenLookAndFeel.h"
```

In the private section of `TrackListComponent` (around line 175), add `PlayScreenLookAndFeel laf;` as the **first** private member, before `juce::ListBox listBox;`:

Old:
```cpp
private:
    // ...
    ViewMode viewMode = ViewMode::FolderView;
    // ...
    juce::ListBox listBox;
```
New — insert `PlayScreenLookAndFeel laf;` directly before `juce::ListBox listBox;`:
```cpp
    PlayScreenLookAndFeel laf;
    juce::ListBox listBox;
```

In the private section of `StylesListComponent` (around line 811), add `PlayScreenLookAndFeel laf;` as the **first** private member, before `juce::OwnedArray<StyleViewComponent> allStyles;`:

Old:
```cpp
private:
    // ...
    juce::OwnedArray<StyleViewComponent> allStyles;
```
New:
```cpp
    PlayScreenLookAndFeel laf;
    juce::OwnedArray<StyleViewComponent> allStyles;
```

- [ ] **Step 2: Apply LAF to folder buttons in `TrackListComponent` constructor**

In `displayGUI.cpp`, the `TrackListComponent` constructor has:
```cpp
    addAndMakeVisible(addButtonFolder);
    addAndMakeVisible(removeButtonFolder);
    addAndMakeVisible(renameButtonFolder);

    addButtonFolder.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    removeButtonFolder.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    renameButtonFolder.setMouseCursor(juce::MouseCursor::PointingHandCursor);
```

After the three `setMouseCursor` lines, add:
```cpp
    addButtonFolder.setLookAndFeel(&laf);
    addButtonFolder.setColour(juce::TextButton::buttonColourId, AppColours::accent7);

    removeButtonFolder.setLookAndFeel(&laf);
    removeButtonFolder.setColour(juce::TextButton::buttonColourId, AppColours::accent6);

    renameButtonFolder.setLookAndFeel(&laf);
    renameButtonFolder.setColour(juce::TextButton::buttonColourId, AppColours::accent4);
```

- [ ] **Step 3: Apply LAF to track buttons in `TrackListComponent::initializeTracksFromList`**

In `displayGUI.cpp`, `TrackListComponent::initializeTracksFromList` creates four buttons then calls `addAndMakeVisible` on each. After the four `setMouseCursor` calls and before the `onClick` lambdas, add:

```cpp
    backButton->setLookAndFeel(&laf);
    backButton->setColour(juce::TextButton::buttonColourId, AppColours::accent2);

    addButton->setLookAndFeel(&laf);
    addButton->setColour(juce::TextButton::buttonColourId, AppColours::accent7);

    removeButton->setLookAndFeel(&laf);
    removeButton->setColour(juce::TextButton::buttonColourId, AppColours::accent6);

    renameButton->setLookAndFeel(&laf);
    renameButton->setColour(juce::TextButton::buttonColourId, AppColours::accent4);
```

The block in `initializeTracksFromList` with all mouse cursor and LAF settings should look like:
```cpp
    backButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    removeButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    renameButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);

    backButton->setLookAndFeel(&laf);
    backButton->setColour(juce::TextButton::buttonColourId, AppColours::accent2);

    addButton->setLookAndFeel(&laf);
    addButton->setColour(juce::TextButton::buttonColourId, AppColours::accent7);

    removeButton->setLookAndFeel(&laf);
    removeButton->setColour(juce::TextButton::buttonColourId, AppColours::accent6);

    renameButton->setLookAndFeel(&laf);
    renameButton->setColour(juce::TextButton::buttonColourId, AppColours::accent4);

    backButton->onClick = [this]() {
        backToFolderView();
    };
    // ... rest of onClick lambdas unchanged
```

- [ ] **Step 4: Update `TrackListComponent::paintListBoxItem` to use dark colours**

Find `paintListBoxItem` in `displayGUI.cpp`:
```cpp
void TrackListComponent::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);
    else
        g.fillAll(juce::Colours::white);

    g.setColour(juce::Colours::black);
    // ...
    g.drawText(label, 5, 0, width - 10, height, juce::Justification::centredLeft);

    g.setColour(juce::Colours::lightgrey);
    g.drawRect(0, 0, width, height, 1);
}
```

Replace the colour lines:
```cpp
void TrackListComponent::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(AppColours::accent2.withAlpha(0.35f));
    else
        g.fillAll(AppColours::panelBg);

    g.setColour(AppColours::titleText);

    juce::String label;
    // ... rest of label logic unchanged ...
    g.drawText(label, 5, 0, width - 10, height, juce::Justification::centredLeft);

    g.setColour(AppColours::separator);
    g.drawRect(0, 0, width, height, 1);
}
```

- [ ] **Step 5: Apply LAF to `StylesListComponent` addButton + update StylesListComponent paint**

In `displayGUI.cpp`, the `StylesListComponent` constructor has:
```cpp
    addAndMakeVisible(addButton);
    addButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
```

After `setMouseCursor`, add:
```cpp
    addButton.setLookAndFeel(&laf);
    addButton.setColour(juce::TextButton::buttonColourId, AppColours::accent7);
```

Then update `StylesListComponent::paint`. Find:
```cpp
void StylesListComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    const int controlBarHeight = 24;

    auto lightColor = juce::Colour::fromRGB(80, 80, 80);
    auto darkColor = juce::Colour::fromRGB(40, 40, 40);

    juce::ColourGradient gradient(lightColor, 0, 0, darkColor, 0, (float)controlBarHeight, false);
    g.setGradientFill(gradient);
    g.fillRect(0, 0, getWidth(), controlBarHeight);

    auto borderColor = juce::Colours::darkgrey.withAlpha(0.3f);
    g.setColour(borderColor);
    g.drawLine(0.0f, (float)(controlBarHeight - 1), (float)getWidth(), (float)(controlBarHeight - 1), 1.0f);
}
```

Replace with:
```cpp
void StylesListComponent::paint(juce::Graphics& g)
{
    g.fillAll(AppColours::panelBg);

    const int controlBarHeight = 24;

    juce::ColourGradient gradient(AppColours::cardBg.brighter(0.05f), 0, 0,
                                  AppColours::cardBg, 0, (float)controlBarHeight, false);
    g.setGradientFill(gradient);
    g.fillRect(0, 0, getWidth(), controlBarHeight);

    g.setColour(AppColours::separator);
    g.drawLine(0.0f, (float)(controlBarHeight - 1), (float)getWidth(), (float)(controlBarHeight - 1), 1.0f);
}
```

- [ ] **Step 6: Update `StyleViewComponent` label colour from cyan to teal**

In `displayGUI.cpp`, find the `StyleViewComponent` constructor where label colour is set:
```cpp
    label.setColour(juce::Label::textColourId, juce::Colours::cyan);
```
Replace with:
```cpp
    label.setColour(juce::Label::textColourId, AppColours::accent3);
```

- [ ] **Step 7: Build and run — open the Display, verify Add folder (emerald), Remove folder (coral), Rename folder (blue) buttons, dark list rows, teal style labels**

- [ ] **Step 8: Commit**

```bash
git add Source/displayGUI.h Source/displayGUI.cpp
git commit -m "feat: apply PlayScreenLookAndFeel to display and track list buttons"
```

---

## Spec Coverage Self-Review

| Spec requirement | Covered by |
|---|---|
| HeaderPanel dark + purple top + teal bottom | Task 3 |
| PlayScreenLookAndFeel class | Task 1 |
| Home, Particle colours, Instruments, Chord helper, Save/Play recording buttons | Task 4 |
| ToggleButton teal tick + rowLabel disabled + titleText label | Task 5 steps 1-4 |
| Section buttons (Intros/Endings/Vars/Fills/Break) LAF | Task 6 |
| Section active colour → teal | Task 7 |
| Display Add folder (emerald), Remove (coral), Rename (blue) | Task 8 step 2 |
| Track Add (emerald), Remove (coral), Rename (blue), Back (purple) | Task 8 step 3 |
| Dark list rows + separator colour | Task 8 step 4 |
| StylesListComponent Add button styled | Task 8 step 5 |
| StylesListComponent paint dark | Task 8 step 5 |
| StyleViewComponent label teal | Task 8 step 6 |
| Loading label "Preparing style..." in onSfzLoadStart | Task 5 step 6 |
| Loading label shown on style switch | Task 5 step 7 |
| Play button label text updated | Task 5 step 5 |
