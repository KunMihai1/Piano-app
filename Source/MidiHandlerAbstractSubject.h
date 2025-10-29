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
 * @class MidiHandlerListener
 * @brief Abstract interface for classes that want to listen to MIDI events.
 *
 * Classes inheriting from MidiHandlerListener can override the virtual
 * methods to receive notifications when MIDI notes are played or MIDI
 * messages are received.
 */
class MidiHandlerListener {
public:
    /** @brief Virtual destructor for proper cleanup of derived classes */
    virtual ~MidiHandlerListener() = default;

    /**
     * @brief Called when a MIDI note-on event is received.
     * @param midiNote The MIDI note number (0-127)
     */
    virtual void noteOnReceived(int midiNote) { (void)midiNote; }

    /**
     * @brief Called when a MIDI note-off event is received.
     * @param midiNote The MIDI note number (0-127)
     */
    virtual void noteOffReceived(int midiNote) { (void)midiNote; }

    /**
     * @brief Called for any incoming MIDI message.
     * @param message The full MIDI message received
     */
    virtual void handleIncomingMessage(const juce::MidiMessage& message) { (void)message; }
};
