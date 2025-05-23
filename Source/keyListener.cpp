/*
  ==============================================================================

    keyListener.cpp
    Created: 23 May 2025 5:20:30pm
    Author:  Kisuke

  ==============================================================================
*/

#include "keyListener.h"

KeyboardListener::KeyboardListener(MidiHandler& midiHandler) : midiHandler{ midiHandler }
{

}

bool KeyboardListener::keyPressed(const juce::KeyPress& key, juce::Component*)
{
    if (!this->isKeyBoardInput)
        return false;

    int midiNote = mapKeyMidi(key);

    auto it = std::find(allPressedKeys.begin(), allPressedKeys.end(), midiNote);

    if (midiNote != -1 && it==allPressedKeys.end())
    {
        allPressedKeys.push_back(midiNote);
        this->midiHandler.noteOnKeyboard(midiNote, 127);
    }

}

bool KeyboardListener::keyStateChanged(bool isKeyDown, juce::Component*)
{

    for (int i = static_cast<int>(allPressedKeys.size()) - 1; i >= 0; i--)
    {
        int note = allPressedKeys[i];
        int keyCode = intToKey[note];
        if (!juce::KeyPress::isKeyCurrentlyDown(keyCode))
        {
            allPressedKeys.erase(allPressedKeys.begin() + i);
            this->midiHandler.noteOffKeyboard(note, 127);
        }

    }
    return false;
}

void KeyboardListener::setIsKeyboardInput(bool state)
{
    this->isKeyBoardInput = state;
}

int KeyboardListener::mapKeyMidi(const juce::KeyPress& key)
{
    switch (key.getKeyCode())
    {
        case 'A'://60
            keyToInt[key.getKeyCode()] = 60;
            intToKey[60] = key.getKeyCode();
            return 60;

        case 'W':
            keyToInt[key.getKeyCode()] = 61;
            intToKey[61] = key.getKeyCode();
            return 61;

        case 'S':
            keyToInt[key.getKeyCode()] = 62;
            intToKey[62] = key.getKeyCode();
            return 62;

        case 'E':
            keyToInt[key.getKeyCode()] = 63;
            intToKey[63] = key.getKeyCode();
            return 63;

        case 'D':
            keyToInt[key.getKeyCode()] = 64;
            intToKey[64] = key.getKeyCode();
            return 64;

        case 'F':
            keyToInt[key.getKeyCode()] = 65;
            intToKey[65] = key.getKeyCode();
            return 65;

        case 'T':
            keyToInt[key.getKeyCode()] = 66;
            intToKey[66] = key.getKeyCode();
            return 66;

        case 'G':
            keyToInt[key.getKeyCode()] = 67;
            intToKey[67] = key.getKeyCode();
            return 67;

        case 'Y':
            keyToInt[key.getKeyCode()] = 68;
            intToKey[68] = key.getKeyCode();
            return 68;

        case 'H':
            keyToInt[key.getKeyCode()] = 69;
            intToKey[69] = key.getKeyCode();
            return 69;

        case 'U':
            keyToInt[key.getKeyCode()] = 70;
            intToKey[70] = key.getKeyCode();
            return 70;

        case 'J':
            keyToInt[key.getKeyCode()] = 71;
            intToKey[71] = key.getKeyCode();
            return 71;

        case 'K': 
            keyToInt[key.getKeyCode()] = 72;
            intToKey[72] = key.getKeyCode();
            return 72;

        default:
            return -1;
    }
}