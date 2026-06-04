# Play Screen UI Improvements — Design Spec
Date: 2026-06-04

## Goal
Restyle all visible controls on the play screen (post-Play-button state) so they share a
cohesive dark visual language consistent with AppColours and the existing overlay component.
Also fix the blank loading overlay that appears when switching styles with the internal SFZ
synth selected.

---

## 1. HeaderPanel Background

**File:** `MainComponent.cpp` → `MainComponent::headerPanel::paint`

Replace the crimson→gold gradient with:
- Base fill: `AppColours::panelBg` (`#1a1a2e`)
- Subtle vertical gradient overlay: `AppColours::accent2` at 18% alpha (top) fading to
  transparent (bottom) — provides just enough purple warmth to read as a distinct bar
- Top edge strip: 3 px solid `AppColours::accent2` (purple `#845ec2`)
- Bottom border: 2 px solid `AppColours::accent3` (teal `#00c9a7`) — acts as the
  visual horizon separating the control bar from the dark note-drawing area below

---

## 2. PlayScreenLookAndFeel — new class

**New file:** `Source/PlayScreenLookAndFeel.h`

Inherits `juce::LookAndFeel_V4`. Applied to all TextButtons on the play screen.

### `drawButtonBackground`
- Corner radius: 6 px rounded rect
- Fill: `AppColours::cardBg` (`#16213e`)
  - Hover: `cardBg.brighter(0.15f)`
  - Press: `cardBg.darker(0.15f)`
- Border: 1.5 px, colour taken from the button's `buttonColourId` (allows role-coding)
  - Default (no explicit colour set): `AppColours::accent2` (purple)

### `getTextButtonFont`
- 13 px, plain weight (replaces the 40 px bold of `CustomLookAndFeel`)

### `drawToggleButton`
- Tick colour when ON: `AppColours::accent3` (teal)
- Tick colour when OFF: `AppColours::rowLabel` (muted `#5e5e7e`)
- Label text colour: `AppColours::titleText` (`#e0e0ff`)

---

## 3. Header TextButtons

**File:** `MainComponent.cpp` init functions + `MainComponent.h` (add LAF member)

Apply `PlayScreenLookAndFeel` to: `homeButton`, `colourSelectorButton`,
`instrumentSelectorButton`, `chordHelperButton`, `saveRecordingButton`,
`playRecordingFileButton`.

All share the default purple (`accent2`) border via
`setColour(juce::TextButton::buttonColourId, AppColours::accent2)`.

---

## 4. ToggleButtons

**File:** `MainComponent.cpp` init functions (`toggleButtonInit`, `toggleHandButtonsInit`,
`annotationInit`)

Change for: `particleToggle`, `noteNumbersAnnotation`, `leftHandInstrumentToggle`,
`rightHandInstrumentToggle`.

- `tickColourId` → `AppColours::accent3` (teal)
- `tickDisabledColourId` → `AppColours::rowLabel`
- Text colour → `AppColours::titleText`

---

## 5. Section Buttons (Intros / Endings / Variations / Fills / Break)

**Files:** `SectionGroupComponent.cpp`, `SectionsComponent.cpp`

- `SectionGroupComponent`: add `PlayScreenLookAndFeel` member; apply to all `TextButton`s
  created in constructor (and the Break button). Set default border to `accent2`.
  Style title `Label` with `AppColours::subtitleText` colour.
- `SectionsComponent::applyChangeColour`: change active fill from
  `juce::Colours::green.withAlpha(0.8f)` → `AppColours::accent3.withAlpha(0.85f)` (teal).

---

## 6. Display Buttons (TrackListComponent + StylesListComponent)

**File:** `displayGUI.cpp`

Add `PlayScreenLookAndFeel` member to `TrackListComponent` and `StylesListComponent`.

Role-coded border colours (set via `setColour(juce::TextButton::buttonColourId, ...)`):

| Button | Border colour |
|---|---|
| Add / Add folder | `AppColours::accent7` (emerald `#26de81`) |
| Remove / Remove folder | `AppColours::accent6` (coral `#fc5c65`) |
| Rename / Rename folder | `AppColours::accent4` (blue `#4b7bec`) |
| Back | `AppColours::accent2` (purple) |

`TrackListComponent::paintListBoxItem`:
- Selected row: `AppColours::accent2.withAlpha(0.35f)` (replaces `lightblue`)
- Unselected row: `AppColours::panelBg` (replaces `white`)
- Row text: `AppColours::titleText`

---

## 7. Loading Overlay — "Preparing style…"

**File:** `MainComponent.cpp`

### Text change
`ensureAudioHandlerReady` — `onSfzLoadStart` callback:
- Change text from `"Configuring instrument..."` → `"Preparing style..."`

### Style-switch path
In `displayInit` lambda (`loadSettingsOnStyleChange`), before `loadSfzForCurrentStyle`:
```
openingAudioLabel.setText("Preparing style...", dontSendNotification);
openingAudioLabel.setVisible(true);
```
Add a 300 ms deferred fallback hide so the label never stays on screen if no SFZ file
actually needed loading (i.e. `onSfzLoadComplete` never fires):
```
juce::Timer::callAfterDelay(300, [safeThis]() {
    if (safeThis) safeThis->openingAudioLabel.setVisible(false);
});
```
`onSfzLoadComplete` hides it immediately when real loading finishes, which wins over the
timer in the normal case.

### Play-button path
Keep the 50 ms timer and existing hide logic. Only change the initial text from
`"Opening Audio Device..."` → `"Preparing session..."`. The style-switch path above
covers the user-reported blank-label case; the play-button path is a separate, minor
flash that does not require restructuring.

---

## Files Changed

| File | Change |
|---|---|
| `Source/PlayScreenLookAndFeel.h` | **New** — shared LookAndFeel class |
| `Source/AppColours.h` | No change (used as-is) |
| `Source/MainComponent.cpp` | Header paint, button inits, loading label |
| `Source/MainComponent.h` | Add `PlayScreenLookAndFeel` member |
| `Source/SectionGroupComponent.cpp` | Apply LAF to section buttons |
| `Source/SectionGroupComponent.h` | Add LAF member |
| `Source/SectionsComponent.cpp` | Active colour → teal |
| `Source/displayGUI.cpp` | Display/track buttons LAF + list colours |
| `Source/displayGUI.h` | Add LAF members to TrackListComponent, StylesListComponent |
