/*
  ==============================================================================

    ValidatorUI.h
    Created: 21 Jul 2025 7:23:52pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

/**
 * @class Validator
 * @brief Provides utility functions to validate MIDI values and strings.
 *
 * This class contains static methods for checking whether integer or double
 * MIDI values and string representations are valid according to MIDI specifications.
 */
class Validator
{
public:
    /**
     * @brief Checks if an integer is a valid MIDI value (0–127).
     * @param value Integer value to check.
     * @return True if the value is between 0 and 127, false otherwise.
     */
    static bool isValidMidiIntValue(int value);
    
    /**
     * @brief Checks if a string contains a valid MIDI integer representation.
     * 
     * Acceptable characters: digits, optional leading '+' or '-'.
     * Rejects malformed strings like "+", "-", "++", "--".
     * @param s The string to validate.
     * @return True if the string is a valid MIDI integer representation.
     */
    static bool isValidMidiIntegerString(const juce::String& s);

     /**
     * @brief Checks if a value is a valid MIDI velocity integer (0–127).
     * @param value The value to check.
     * @return True if the value is valid, false otherwise.
     */
    static bool isValidMidiIntegerVelocities(double value);

    /**
     * @brief Checks if a double value is valid for MIDI usage.
     * @param value Double value to check.
     * @return True if the value is considered valid (currently always true).
     */
    static bool isValidMidiDoubleValue(double value);

    /**
     * @brief Validates a double value representing a time delta relative to MIDI events.
     * 
     * Ensures the value does not produce negative timestamps and respects event order.
     * @param value The delta value to check.
     * @param currentTimeStamp Timestamp of the current event.
     * @param previousTimeStamp Timestamp of the previous event.
     * @return True if the value is valid in the context of the timestamps.
     */
    static bool isValidMidiDoubleValueTimeStamps(double value, double currentTimeStamp=-1, double previousTimeStamp=-1);

    /**
     * @brief Checks if a string contains a valid MIDI double representation.
     * 
     * Acceptable characters: digits, '+', '-', and one optional decimal point.
     * Rejects malformed strings like "+", "-", "+.", "-.", "++", "--", "..".
     * @param s The string to validate.
     * @return True if the string is a valid MIDI double representation.
     */
    static bool isValidMidiDoubleString(const juce::String& s);

private:

};
