/*
  ==============================================================================

    HelperFunction.h
    Created: 12 Oct 2025 8:32:50pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


/**
 * @brief Utility class for converting between MIDI note numbers and note name strings.
 *
 * @details
 * Provides static helper functions to convert integers (0–127) to standard
 * note names with octave (e.g., 60 -> "C4") and vice versa.
 */
class MapHelper {
public:

    /**
     * @brief Converts a MIDI note number to its string representation.
     *
     * @param note MIDI note number (0–127)
     * @return Note name as a string (e.g., "C4"). Returns "Invalid" if the input is out of range.
     *
     * @details
     * The note names follow standard western notation: C, C#, D, D#, E, F, F#, G, G#, A, A#, B.
     * The octave is calculated as (note / 12) - 1.
     */
    static juce::String intToStringNote(int note);

     /**
     * @brief Converts a note name string to its corresponding MIDI note number.
     *
     * @param noteString Note string (e.g., "C4", "D#3"). Can include leading/trailing spaces.
     * @return MIDI note number (0–127). Returns -1 if the string is invalid or out of range.
     *
     * @details
     * The function trims whitespace and is case-insensitive.
     * It parses the note part (C, C#, D, etc.) and the octave part separately.
     */
    static int stringToIntNote(juce::String);

private:
};
