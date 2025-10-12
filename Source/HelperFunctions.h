/*
  ==============================================================================

    HelperFunction.h
    Created: 12 Oct 2025 8:32:50pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


class MapHelper {
public:
    static juce::String intToStringNote(int note);
    static int stringToIntNote(juce::String);

private:
};