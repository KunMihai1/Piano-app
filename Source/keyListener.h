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

class KeyboardListener : public juce::KeyListener
{
public:
    KeyboardListener(MidiHandler& midiHandler);

    bool keyPressed(const juce::KeyPress& key, juce::Component*) override;
    bool keyStateChanged(bool isKeyDown, juce::Component*) override;
    void setIsKeyboardInput(bool state);

private:
    MidiHandler& midiHandler;

    int mapKeyMidi(const juce::KeyPress& key);
    std::unordered_map<int, int> keyToInt;  
    std::unordered_map<int, int> intToKey;
    bool isKeyBoardInput = false;
    std::vector<int> allPressedKeys;
};