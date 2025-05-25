/*
  ==============================================================================

    SubjectAbstract.h
    Created: 25 May 2025 11:24:57am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class MidiHandlerListener {
public:
    virtual ~MidiHandlerListener()=default;
    virtual void noteOnReceived(int midiNote) { (void)midiNote; }
    virtual void noteOffReceived(int midiNote) { (void)midiNote; }
    virtual void handleIncomingMessage(const juce::MidiMessage& message) { (void)message; }
};