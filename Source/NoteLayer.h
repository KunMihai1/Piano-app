/*
  ==============================================================================

    NoteLayer.h
    Created: 20 May 2025 1:36:54pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include "MidiHandler.h"
#include "KeyboardUI.h"


class NoteLayer : public juce::Component, public MidiHandler::Listener, public juce::Timer
{
public:
    NoteLayer(KeyboardUI& referenceKeyboard);
    ~NoteLayer();

    void paint(juce::Graphics& g) override;

    void noteOnReceived(int midiNote) override;
    void noteOffReceived(int midiNote) override;

private:
    KeyboardUI& keyBoardUI;

    void visibilityChanged() override;
    void timerCallback() override;

    struct AnimatedNote {
        juce::Rectangle<int> bounds = { 0,0,0,0 };
        float alpha = 1.0f;
        float velocityY = -50.0f;
        bool isFalling = false;
    };

    std::unordered_map<int, AnimatedNote> activeNotes;
    std::vector<AnimatedNote> fallingNotes;
};