/*
  ==============================================================================

    ValidatorUI.h
    Created: 21 Jul 2025 7:23:52pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class Validator
{
public:
    static bool isValidMidiIntValue(int value);

    static bool isValidMidiIntegerString(const juce::String& s);

    static bool isValidMidiIntegerVelocities(double value);

    static bool isValidMidiDoubleValue(double value);

    static bool isValidMidiDoubleValueTimeStamps(double value, double currentTimeStamp=-1, double previousTimeStamp=-1);

    static bool isValidMidiDoubleString(const juce::String& s);

private:

};