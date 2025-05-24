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
    return false;
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

bool KeyboardListener::getIsKeyboardInput()
{
    return isKeyBoardInput;
}

void KeyboardListener::resetState()
{
    this->intToKey.clear();
    this->keyToInt.clear();
    this->allPressedKeys.clear();
}

int KeyboardListener::getStartNoteKeyboardInput()
{
    return startNoteKeyboardInput;
}

void KeyboardListener::setStartNoteKeyboardInput(int value)
{
    startNoteKeyboardInput = value;
}

int KeyboardListener::getFinishNoteKeyboardInput()
{
    return finishNoteKeyboardInput;
}

void KeyboardListener::setFinishNoteKeyboardInput(int value)
{
    finishNoteKeyboardInput = value;
}

int KeyboardListener::mapKeyMidi(const juce::KeyPress& key)
{
    switch (key.getKeyCode())
    {
        case 'A'://60
            if (startNoteKeyboardInput >= 24)
            {
                keyToInt[key.getKeyCode()] = startNoteKeyboardInput;
                intToKey[startNoteKeyboardInput] = key.getKeyCode();
                return startNoteKeyboardInput;
            }
            else return -1;

        case 'W':
            if (startNoteKeyboardInput >= 24)
            {
                keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 1;
                intToKey[startNoteKeyboardInput + 1] = key.getKeyCode();
                return startNoteKeyboardInput + 1;
            }
            else return -1;

        case 'S':
            if (startNoteKeyboardInput >= 24)
            {
                keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 2;
                intToKey[startNoteKeyboardInput + 2] = key.getKeyCode();
                return startNoteKeyboardInput + 2;
            }
            else return -1;

        case 'E':
            if (startNoteKeyboardInput >= 24)
            {
                keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 3;
                intToKey[startNoteKeyboardInput + 3] = key.getKeyCode();
                return startNoteKeyboardInput + 3;
            }
            else return -1;

        case 'D':
            if (startNoteKeyboardInput >= 24)
            {
                keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 4;
                intToKey[startNoteKeyboardInput + 4] = key.getKeyCode();
                return startNoteKeyboardInput + 4;
            }
            else return -1;

        case 'F':
            if (startNoteKeyboardInput >= 24)
            {
                keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 5;
                intToKey[startNoteKeyboardInput + 5] = key.getKeyCode();
                return startNoteKeyboardInput + 5;
            }
            else return -1;

        case 'T':
            if (startNoteKeyboardInput >= 24)
            {
                keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 6;
                intToKey[startNoteKeyboardInput + 6] = key.getKeyCode();
                return startNoteKeyboardInput + 6;
            }
            else return -1;

        case 'G':
            if (startNoteKeyboardInput >= 24)
            {
                keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 7;
                intToKey[startNoteKeyboardInput + 7] = key.getKeyCode();
                return startNoteKeyboardInput + 7;
            }
            else return -1;

        case 'Y':
            if (startNoteKeyboardInput >= 24)
            {
                keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 8;
                intToKey[startNoteKeyboardInput + 8] = key.getKeyCode();
                return startNoteKeyboardInput + 8;
            }
            else return -1;

        case 'H':
            keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 9;
            intToKey[startNoteKeyboardInput + 9] = key.getKeyCode();
            return startNoteKeyboardInput + 9;

        case 'U':
            keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 10;
            intToKey[startNoteKeyboardInput + 10] = key.getKeyCode();
            return startNoteKeyboardInput + 10;

        case 'J':
            keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 11;
            intToKey[startNoteKeyboardInput + 11] = key.getKeyCode();
            return startNoteKeyboardInput + 11;

        case 'K': 
            keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 12;
            intToKey[startNoteKeyboardInput + 12] = key.getKeyCode();
            return startNoteKeyboardInput + 12;
       
        case 'O':
            if (finishNoteKeyboardInput <= 101)
            {
                keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 13;
                intToKey[startNoteKeyboardInput + 13] = key.getKeyCode();
                return startNoteKeyboardInput + 13;
            }
            else return -1;

        case 'L':
            if (finishNoteKeyboardInput <= 101)
            {
                keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 14;
                intToKey[startNoteKeyboardInput + 14] = key.getKeyCode();
                return startNoteKeyboardInput + 14;
            }
            else return -1;

        case 'P':
            if (finishNoteKeyboardInput <= 101)
            {
                keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 15;
                intToKey[startNoteKeyboardInput + 15] = key.getKeyCode();
                return startNoteKeyboardInput + 15;
            }
            else return -1;
            
        case ';':
            if (finishNoteKeyboardInput <= 101)
            {
                keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 16;
                intToKey[startNoteKeyboardInput + 16] = key.getKeyCode();
                return startNoteKeyboardInput + 16;
            }
            else return -1;

        case '\'':
            if (finishNoteKeyboardInput <= 101)
            {
                keyToInt[key.getKeyCode()] = startNoteKeyboardInput + 17;
                intToKey[startNoteKeyboardInput + 17] = key.getKeyCode();
                return startNoteKeyboardInput + 17;
            }
            else return -1;

        default:
            return -1;
    }
}