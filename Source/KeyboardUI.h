/*
  ==============================================================================

    keyboardUI.h
    Created: 26 Mar 2025 9:45:11pm
    Author:  Kisuke

  ==============================================================================
*/
#include "MidiHandler.h"
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#pragma once

class KeyboardUI: public juce::Component, public MidiHandler::Listener, public juce::Timer
{
public:
    KeyboardUI(MidiHandler& midiHandler);
    ~KeyboardUI();
    void paint(juce::Graphics& g) override;
    void set_min_and_max(const int min, const int max);
    
    void noteOnReceived(int midiNote) override;
    void noteOffReceived(int midiNote) override;

private:
    void visibilityChanged() override;
    void timerCallback() override;
    struct note {
        juce::Rectangle<int> bounds={0,0,0,0};
        bool isActive=false;
        std::string type="";
    };
    bool isDrawn = false;

    
    struct animatedNote {
        juce::Rectangle<int> bounds={0,0,0,0};
        float alpha=1.0f;
        bool isFalling = false;
    };

    MidiHandler& midiHandler;
    std::unordered_map<int, note> keys;
    std::unordered_map<int, animatedNote> activeNotes;
    std::vector<animatedNote> fallingNotes;

    int min_draw=0, max_draw=127;
};