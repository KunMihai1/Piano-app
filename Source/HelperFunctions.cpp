/*
  ==============================================================================

    HelperFunction.cpp
    Created: 12 Oct 2025 8:32:50pm
    Author:  Kisuke

  ==============================================================================
*/

#include "HelperFunctions.h"

juce::String MapHelper::intToStringNote(int note)
{
    static const juce::StringArray noteNames{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    if (note < 0 || note > 127)
        return "Invalid";

    int octave = (note / 12) - 1;
    juce::String name = noteNames[note % 12] + juce::String(octave);

    return name;

}

int MapHelper::stringToIntNote(juce::String noteString)
{

    noteString = noteString.trim().toUpperCase();
    static const juce::StringArray noteNames{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    int firstDigitIndex = -1;
    for (int i = 0; i < noteString.length(); ++i)
    {
        if (juce::CharacterFunctions::isDigit(noteString[i]) || noteString[i] == '-')
        {
            firstDigitIndex = i;
            break;
        }
    }

    if (firstDigitIndex == -1)
        return -1; 

    juce::String notePart = noteString.substring(0, firstDigitIndex);
    juce::String octavePart = noteString.substring(firstDigitIndex);

    int noteIndex = noteNames.indexOf(notePart);
    if (noteIndex == -1)
        return -1; 

    int octave = octavePart.getIntValue();
    int midiNote = (octave + 1) * 12 + noteIndex;

    if (midiNote < 0 || midiNote > 127)
        return -1;

    return midiNote;
}