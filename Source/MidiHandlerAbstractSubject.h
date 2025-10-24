/*
  ==============================================================================

    SubjectAbstract.h
    Created: 25 May 2025 11:24:57am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/**
 * @brief Interface for objects that want to listen to MIDI events from a MidiHandler.
 * 
 * Classes that implement MidiHandlerListener can receive notifications for
 * MIDI note events and other incoming MIDI messages.
 */
class MidiHandlerListener {
public:
    /**@brief Virtual destructor */
    virtual ~MidiHandlerListener()=default;

    /**
     * @brief Called when a MIDI note-on event is received.
     * @param midiNote The MIDI note number (0-127) that was pressed.
     * 
     * Override this method to respond to note-on events.
     */
    virtual void noteOnReceived(int midiNote) { (void)midiNote; }

    /**
     * @brief Called when a MIDI note-off event is received.
     * @param midiNote The MIDI note number (0-127) that was released.
     * 
     * Override this method to respond to note-off events.
     */
    virtual void noteOffReceived(int midiNote) { (void)midiNote; }

    /**
     * @brief Called for all incoming MIDI messages.
     * @param message The full juce::MidiMessage received.
     * 
     * Override this method to handle any kind of MIDI message.
     */
    virtual void handleIncomingMessage(const juce::MidiMessage& message) { (void)message; }
};
