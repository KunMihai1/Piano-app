/*
  ==============================================================================

    keyListener.h
    Created: 23 May 2025 5:20:30pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "MidiHandler.h"


/**
 * @class KeyboardListener
 * @brief Handles computer keyboard input and converts it into MIDI note events.
 *
 * This class maps QWERTY keyboard keys to MIDI note numbers and triggers
 * noteOn/noteOff events through a MidiHandler. It tracks currently pressed keys,
 * prevents duplicate note triggers, and supports customizable MIDI ranges.
 */
class KeyboardListener : public juce::KeyListener
{
public:
    /**
     * @brief Creates a KeyboardListener linked to a MIDI handler.
     * @param midiHandler Reference to the MIDI handler used to send input events.
     */
    KeyboardListener(MidiHandler& midiHandler);

    /**
     * @brief Handles key press events and triggers MIDI Note-On messages.
     *
     * Prevents duplicate triggers if the key is already held.
     *
     * @param key The key press event object.
     * @param originatingComponent (unused) Component receiving the keyboard input.
     * @return False to allow JUCE to continue normal key processing.
     */
    bool keyPressed(const juce::KeyPress& key, juce::Component*) override;

    /**
     * @brief Handles key state changes and triggers MIDI Note-Off messages.
     *
     * Detects released keys by comparing active key states.
     *
     * @param isKeyDown Whether a key is currently pressed (unused).
     * @param originatingComponent (unused).
     * @return False to allow JUCE to continue normal processing.
     */
    bool keyStateChanged(bool isKeyDown, juce::Component*) override;

    /**
     * @brief Enables or disables MIDI keyboard input listening.
     * @param state True to activate keyboard input, false to disable it.
     */
    void setIsKeyboardInput(bool state);

    /**
     * @brief Checks whether keyboard input mode is active.
     * @return True if handling keyboard input, otherwise false.
     */
    bool getIsKeyboardInput();

    /**
     * @brief Clears all stored keyboard input state and key mappings.
     */
    void resetState();

    /**
     * @brief Gets the lowest MIDI note allowed for keyboard input.
     * @return Starting MIDI note value.
     */
    int getStartNoteKeyboardInput();

    /**
     * @brief Sets the lowest MIDI note allowed for keyboard input.
     * @param value Starting MIDI note value.
     */
    void setStartNoteKeyboardInput(int value);

    /**
     * @brief Gets the highest MIDI note allowed for keyboard input.
     * @return Last playable MIDI note value.
     */
    int getFinishNoteKeyboardInput();

    /**
     * @brief Sets the highest MIDI note allowed for keyboard input.
     * @param value Last playable MIDI note value.
     */
    void setFinishNoteKeyboardInput(int value);

private:
    MidiHandler& midiHandler;
    int startNoteKeyboardInput = 60;
    int finishNoteKeyboardInput = 77;
    int mapKeyMidi(const juce::KeyPress& key);
    std::unordered_map<int, int> keyToInt;  
    std::unordered_map<int, int> intToKey;
    bool isKeyBoardInput = false;
    std::vector<int> allPressedKeys;
};
