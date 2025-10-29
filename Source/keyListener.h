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
 * @brief Listens to computer keyboard input and converts it to MIDI notes.
 *
 * Handles:
 *   - Key press and release events
 *   - Mapping keyboard keys to MIDI note numbers
 *   - Tracking currently pressed keys
 *   - Configurable start and end notes for the keyboard input range
 *
 * Inherits from juce::KeyListener.
 */
class KeyboardListener : public juce::KeyListener
{
public:
    /**
     * @brief Constructor.
     * @param midiHandler Reference to the MidiHandler to send note events.
     */
    KeyboardListener(MidiHandler& midiHandler);

    /**
     * @brief Called when a key is pressed.
     * @param key The key that was pressed.
     * @param component The component that received the key press.
     * @return true if the event was consumed, false otherwise.
     */
    bool keyPressed(const juce::KeyPress& key, juce::Component*) override;

    /**
     * @brief Called when a key state changes (pressed/released).
     * @param isKeyDown true if a key is pressed, false if released.
     * @param component The component that received the change.
     * @return true if the event was consumed, false otherwise.
     */
    bool keyStateChanged(bool isKeyDown, juce::Component*) override;

    /**
     * @brief Enables or disables keyboard input for MIDI.
     * @param state true to enable, false to disable.
     */
    void setIsKeyboardInput(bool state);

    /**
     * @brief Checks whether keyboard input is currently enabled.
     * @return true if enabled, false otherwise.
     */
    bool getIsKeyboardInput();

    /**
     * @brief Resets the internal state of the listener.
     * Clears all key mappings and pressed key records.
     */
    void resetState();

    /**
     * @brief Gets the starting MIDI note for keyboard input.
     * @return The starting note number.
     */
    int getStartNoteKeyboardInput();

    /**
     * @brief Sets the starting MIDI note for keyboard input.
     * @param value The new starting note number.
     */
    void setStartNoteKeyboardInput(int value);

    /**
     * @brief Gets the ending MIDI note for keyboard input.
     * @return The ending note number.
     */
    int getFinishNoteKeyboardInput();

    /**
     * @brief Sets the ending MIDI note for keyboard input.
     * @param value The new ending note number.
     */
    void setFinishNoteKeyboardInput(int value);

private:
    /**
     * @brief Maps a computer key to a MIDI note number.
     * @param key The key pressed.
     * @return MIDI note number, or -1 if the key is not mapped.
     */
    int mapKeyMidi(const juce::KeyPress& key);

    // Member variables
    MidiHandler& midiHandler;                  /**< Reference to the MIDI handler */
    int startNoteKeyboardInput = 60;          /**< Starting MIDI note number */
    int finishNoteKeyboardInput = 77;         /**< Ending MIDI note number */
    std::unordered_map<int, int> keyToInt;    /**< Map from key codes to MIDI notes */
    std::unordered_map<int, int> intToKey;    /**< Map from MIDI notes to key codes */
    bool isKeyBoardInput = false;             /**< Whether keyboard input is active */
    std::vector<int> allPressedKeys;          /**< Tracks currently pressed MIDI notes */
};
