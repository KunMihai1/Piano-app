#pragma once
#include <JuceHeader.h>
#include "ArrangerStyleFile.h"

class ArrangerStyleIOHelper
{
public:
    static void saveToFile (const juce::File& file, const ArrangerStyleFile& style);
    static bool loadFromFile (const juce::File& file, ArrangerStyleFile& out, juce::String& error);
};
