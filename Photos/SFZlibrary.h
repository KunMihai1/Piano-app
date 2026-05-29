/*
  ==============================================================================

    SFZlibrary.h
    Created: 29 May 2026 4:13:50pm
    Author:  Oricum

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct SFZLibraryEntry
{
    juce::String name;
    juce::String path;
    juce::File sfzFile;
    int instrumentNumber;
};

class SFZLibraryManager
{
public:

};