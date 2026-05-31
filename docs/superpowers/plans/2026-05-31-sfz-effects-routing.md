# SFZ Effects Routing Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Route effect CC messages (volume, reverb, pan, etc.) through to the internal SFZ synthesisers so that the sound-settings window actually affects audio output.

**Architecture:** A new `injectCC()` method on `MidiHandler` drops a controller event directly into `incomingMidiMessages`, which is already the buffer the `AudioHandler` reads each audio callback. Two call-sites need updating: `sendEffectsBeforePlaying()` (play-start / style-change) and the real-time `saveCallback` in `setCallBacksForEffectWindow()`.

**Tech Stack:** JUCE (C++17), sfzero library, existing `MidiHandler` / `AudioHandler` / `MainComponent`.

---

## Current Architecture – Read This First

### How sound flows today

```
Keyboard input
  → MidiHandler::handleIncomingMidiMessage()
    → incomingMidiMessages (MidiBuffer, guarded by midiMutex)
      → AudioHandler::audioDeviceIOCallbackWithContext()
        → split by MIDI channel
          → sfzSynths[ch-1].renderNextBlock()   ← actual audio
```

### Channel assignment for live keyboard playing
- **Channel 1**  = left-hand instrument (notes below `rightHandBoundSetting`)
- **Channel 16** = right-hand instrument (notes at or above `rightHandBoundSetting`)
- Both channels have their own `sfzero::Synth` in `AudioHandler::sfzSynths[16]`

### SFZ mapping (already works correctly)
- Stored as: `(styleId, programNumber) → SFZ file path`  (`SFZLibraryManager::styleMappings`)
- On instrument select / play-start, `MainComponent` calls:
  ```cpp
  audioHandler->loadSfz(sfzManager.getSfzForStyleInstrument(styleId, leftProg),  1);
  audioHandler->loadSfz(sfzManager.getSfzForStyleInstrument(styleId, rightProg), 16);
  ```
- **Two different SFZ files can play simultaneously** – one per hand, one per channel. This is already implemented and working.

### What is broken: effects CC never reach sfzSynths

`sendEffectsBeforePlaying()` has an early return if `!MIDIDevice.isOpenOUT()`. When using audio output, MIDI OUT is not open, so the function returns immediately and **no CC messages are sent anywhere**.

Even when MIDI OUT is open, `MIDIDevice.sendMidiCC()` routes to `currentDeviceUSEDout` (hardware), **not** into the `incomingMidiMessages` buffer that `AudioHandler` reads. So the sfzSynths have never received a volume, reverb, pan, or any other CC message.

The same problem exists for real-time knob changes in `setCallBacksForEffectWindow()`:
```cpp
if (!playButton.isVisible())   // i.e., we are in play mode
    MIDIDevice.sendMidiCC(currentChannel, ccNumber, value);  // only to hardware
```

---

## File Map

| File | Change |
|------|--------|
| `Source/MidiHandler.h` | Declare `injectCC(int channel, int ccNumber, int value)` |
| `Source/MidiHandler.cpp` | Implement `injectCC` – thread-safe, adds controller event to `incomingMidiMessages` |
| `Source/MainComponent.cpp` | Fix `sendEffectsBeforePlaying()` + `setCallBacksForEffectWindow()` saveCallback |

---

## Task 1: Add `injectCC` to `MidiHandler`

**Files:**
- Modify: `Source/MidiHandler.h`
- Modify: `Source/MidiHandler.cpp`

- [ ] **Step 1: Declare the method in the header**

Open `Source/MidiHandler.h`. In the `MidiHandler` class, after the existing `allOffKeyboard()` declaration (around line 396), add:

```cpp
void injectCC(int channel, int ccNumber, int value);
```

- [ ] **Step 2: Implement it in the .cpp**

Open `Source/MidiHandler.cpp`. After the `allOffKeyboard()` implementation (around line 891), add:

```cpp
void MidiHandler::injectCC(int channel, int ccNumber, int value)
{
    const juce::ScopedLock lock(midiMutex);
    auto msg = juce::MidiMessage::controllerEvent(
        channel,
        ccNumber,
        juce::jlimit(0, 127, value));
    incomingMidiMessages.addEvent(msg, 0);
}
```

- [ ] **Step 3: Build to verify it compiles**

Run: `cmake --build . --config Debug` (or your IDE build shortcut)  
Expected: zero new errors.

- [ ] **Step 4: Commit**

```bash
git add Source/MidiHandler.h Source/MidiHandler.cpp
git commit -m "feat: add MidiHandler::injectCC to route CC messages to sfzSynths"
```

---

## Task 2: Fix `sendEffectsBeforePlaying()` for audio output

**Files:**
- Modify: `Source/MainComponent.cpp:808-852`

This function currently bails if `!MIDIDevice.isOpenOUT()`. It should also fire when audio output is open, routing CC via `midiHandler.injectCC()`.

- [ ] **Step 1: Replace the function body**

Find the current `sendEffectsBeforePlaying()` at line 808.  
Replace the **entire function** with:

```cpp
void MainComponent::sendEffectsBeforePlaying()
{
    const bool hasOut   = MIDIDevice.isOpenOUT();
    const bool hasAudio = MIDIDevice.isOpenAudioOUT();

    if (!hasOut && !hasAudio)
        return;

    struct EffectCC
    {
        int ccNumber;
        std::function<int(int)> getter;
    };

    std::vector<EffectCC> effects = {
        { 74, [&](int ch) { return MIDIDevice.getBrightness(ch); } },
        { 11, [&](int ch) { return MIDIDevice.getExpression(ch); } },
        { 93, [&](int ch) { return MIDIDevice.getChorus(ch); } },
        { 71, [&](int ch) { return MIDIDevice.getResonance(ch); } },
        { 64, [&](int ch) { return MIDIDevice.getSustainToggle(ch) ? 127 : 0; } },
        { 73, [&](int ch) { return MIDIDevice.getAttack(ch); } },
        { 75, [&](int ch) { return MIDIDevice.getDecay(ch); } },
        { 72, [&](int ch) { return MIDIDevice.getRelease(ch); } },
        { 1,  [&](int ch) { return MIDIDevice.getVibrato(ch); } },
        { 94, [&](int ch) { return MIDIDevice.getDelay(ch); } },
        { 10, [&](int ch) { return MIDIDevice.getPan(ch); } },
        { 91, [&](int ch) { return MIDIDevice.getReverb(ch); } },
        { 7,  [&](int ch) { return MIDIDevice.getVolume(ch); } },
        { 80, [&](int ch) { return MIDIDevice.getDistortion(ch); } },
        { 76, [&](int ch) { return MIDIDevice.getFilterTrack(ch); } },
        { 92, [&](int ch) { return MIDIDevice.getTremolo(ch); } },
        { 95, [&](int ch) { return MIDIDevice.getRandomMod(ch); } }
    };

    for (int ch : { 1, 16 })
    {
        for (auto& effect : effects)
        {
            const int value = effect.getter(ch);
            if (hasOut)
                MIDIDevice.sendMidiCC(ch, effect.ccNumber, value);
            if (hasAudio)
                midiHandler.injectCC(ch, effect.ccNumber, value);
        }
    }
}
```

- [ ] **Step 2: Build and verify it compiles**

Run your build. Expected: no errors.

- [ ] **Step 3: Manual smoke test**

1. Open the app, select an audio output device (not MIDI out).
2. Assign an SFZ to an instrument and press Play.
3. In the sound-effects window, lower the **volume** knob for channel 1 to a low value, then press Play again.
4. Play some notes — volume should be noticeably lower than the default.

- [ ] **Step 4: Commit**

```bash
git add Source/MainComponent.cpp
git commit -m "fix: route effect CCs to sfzSynths when using audio output"
```

---

## Task 3: Fix real-time CC injection from the effects window

**Files:**
- Modify: `Source/MainComponent.cpp:714-717` (inside `setCallBacksForEffectWindow`)

The real-time callback currently sends CC only to MIDI hardware. It should also inject into the sfzSynths while in play mode.

- [ ] **Step 1: Find the block inside `saveCallback`**

In `setCallBacksForEffectWindow()` (around line 714), find:

```cpp
        if (!playButton.isVisible())
        {
            MIDIDevice.sendMidiCC(currentChannel, ccNumber, value);
        }
```

- [ ] **Step 2: Replace with dual-path routing**

```cpp
        if (!playButton.isVisible())
        {
            if (MIDIDevice.isOpenOUT())
                MIDIDevice.sendMidiCC(currentChannel, ccNumber, value);
            if (MIDIDevice.isOpenAudioOUT())
                midiHandler.injectCC(currentChannel, ccNumber, value);
        }
```

- [ ] **Step 3: Build and manual test**

1. Press Play (audio output mode), play some notes.
2. While notes are playing, turn the **reverb** and **volume** knobs.
3. You should hear the effect change immediately without needing to stop and restart.

- [ ] **Step 4: Commit**

```bash
git add Source/MainComponent.cpp
git commit -m "fix: inject real-time effect CC into sfzSynths during audio playback"
```

---

## Notes & Discussion Points

### Which CCs will actually do something in sfzero?

`juce::Synthesiser` (base class of `sfzero::Synth`) natively handles:
- **CC 7** – Volume ✓
- **CC 10** – Pan ✓
- **CC 64** – Sustain ✓

Other CCs (reverb, chorus, brightness, envelope etc.) are passed to voice `controllerMoved()` callbacks. sfzero maps some of these to SFZ opcodes (`ampeg_release`, `cutoff`, `lfo_*`, etc.) **if the SFZ file defines them**. If the SFZ file does not define any CC mappings, those knobs will have no audible effect through SFZ — but volume, pan, and sustain will always work.

### Multiple SFZ files — already supported

There is NO code change needed here. The architecture already supports:
- Left hand → channel 1 → its own `sfzSynths[0]`
- Right hand → channel 16 → its own `sfzSynths[15]`

Each hand loads whichever SFZ file is mapped to `(currentStyleId, programNumber)`. Select a different instrument per hand and each gets a completely different SFZ file. Both render simultaneously in the audio callback.

### Track playback and SFZ

Currently `MultipleTrackPlayer` sends MIDI to the external MIDI output device only. Track data does NOT flow through `AudioHandler`/sfzSynths. If you later want tracks to play through SFZ, that needs a separate `injectMidiFromTrack(MidiBuffer)` path into `MidiHandler`. That is out of scope for this plan.

### SFZ assignment is per (style × instrument number)

`SFZLibraryManager::styleMappings[styleId][programNumber] = entryId`

Meaning: the same program number 0 ("Acoustic Grand Piano") can map to a **different SFZ file** in Style A vs Style B. Changing styles automatically re-loads the correct SFZ because `loadSettings()` calls `audioHandler->loadSfz(...)` via the play-start / style-change flow.
