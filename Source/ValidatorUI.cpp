    /*
  ==============================================================================

    ValidatorUI.cpp
    Created: 21 Jul 2025 7:23:52pm
    Author:  Kisuke

  ==============================================================================
*/

#include "ValidatorUI.h"

bool Validator::isValidMidiIntValue(int value)
{
    if (value < 0 || value > 127)
        return false;


    return true;
}

bool Validator::isValidMidiDoubleValue(double value)
{
    return true;
}

bool Validator::isValidMidiDoubleValueTimeStamps(double value, double currentTimeStamp, double previousTimeStamp, double nextTimeStamp)
{
    if (!isValidMidiDoubleValue(value))
        return false;

    double newTime = currentTimeStamp + value;
    if (newTime < 0.0)
        return false;

    if (newTime - previousTimeStamp < 0.0)
        return false;

    if (newTime - nextTimeStamp > 0.0)
        return false;

    return true;
}

bool Validator::isValidMidiIntegerString(const juce::String& s)
{
    if(!s.containsOnly("0123456789-+"))
        return false;

    if (s.contains("-") && s[0] != '-')
        return false;

    if (s.contains("+") && s[0] != '+')
        return false;

    if (s.contains("++") || s.contains("--"))
        return false;

    if (s == "+" || s == "-")
        return false;

    return true;
}

bool Validator::isValidMidiIntegerVelocities(double value)
{
    if (!isValidMidiIntValue(value))
        return false;

    return true;
}

bool Validator::isValidMidiDoubleString(const juce::String& s)
{
    if (!s.containsOnly("0123456789.+-"))
        return false;

    if (s.contains("-") && s[0] != '-')
        return false;

    if (s.contains("+") && s[0] != '+')
        return false;

    if (s.contains("++") || s.contains("--") || s.contains(".."))
        return false;

    if (s == "+" || s == "-" || s=="+." || s=="-.")
        return false;

    return true;
}
