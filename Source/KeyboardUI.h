/*
  ==============================================================================

    NoteLayer.h
    Created: 20 May 2025 1:20:49am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include "MidiHandler.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <unordered_map>
#include <vector>


/**
 * @brief A keyboard UI component that visualizes active MIDI notes.
 *
 * This class listens to a MidiHandler and displays a piano keyboard.
 * Notes received via the MidiHandler will highlight the corresponding keys.
 */
class KeyboardUI : public juce::Component, public MidiHandlerListener
{
public:

     /**
     * @brief Constructs a KeyboardUI bound to a MidiHandler.
     * @param midiHandler Reference to the MidiHandler to receive note events from.
     */
    KeyboardUI(MidiHandler& midiHandler);

    /**
     * @brief Destructor.
     */
    ~KeyboardUI();


    /**
     * @brief Paints the keyboard UI.
     * @param g Graphics context to use for painting.
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Sets the minimum and maximum MIDI notes to display.
     * @param min Minimum MIDI note.
     * @param max Maximum MIDI note.
     */
    void set_min_and_max(const int min, const int max);

    /**
     * @brief Gets the minimum MIDI note currently displayed.
     * @return Minimum MIDI note.
     */
    int get_min();

    /**
     * @brief Gets the maximum MIDI note currently displayed.
     * @return Maximum MIDI note.
     */
    int get_max();


    /**
     * @brief Resets all keys to an inactive state.
     */
    void resetStateActiveNotes();


    /**
     * @brief Called when a note-on MIDI event is received.
     * @param midiNote MIDI note number that was pressed.
     *//
    void noteOnReceived(int midiNote) override;

    /**
     * @brief Called when a note-off MIDI event is received.
     * @param midiNote MIDI note number that was released.
     */
    void noteOffReceived(int midiNote) override;

    /**
     * @brief Sets whether the keyboard has been fully drawn or not.
     * @param state True if the keyboard has been drawn.
     */
    void setIsDrawn(bool state);

private:
    friend class NoteLayer;



    /**
     * @brief Paints the piano keyboard with all keys.
     * @param g Graphics context to use.
     */
    void paintKeyboard(juce::Graphics& g);

    bool isDrawn = false;
    struct note {
        juce::Rectangle<int> bounds = { 0,0,0,0 };
        bool isActive = false;
        std::string type = "";
    };

    MidiHandler& midiHandler;
    std::unordered_map<int, note> keys;

    int min_draw = 0, max_draw = 127;
};
